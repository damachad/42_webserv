/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 13:21:15 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/10/02 14:42:19 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "PostResponse.hpp"

#include <sstream>
#include <sys/stat.h>


#include "Helpers.hpp"
#include "CGI.hpp"

static unsigned short response_status = OK;

PostResponse::PostResponse(const Server &server, HTTP_Request &request,
						   int client_fd, int epoll_fd)
	: AResponse(server, request), _client_fd(client_fd), _epoll_fd(epoll_fd) {}

PostResponse::PostResponse(const PostResponse &src) : AResponse(src) {}

PostResponse::~PostResponse() {}

unsigned short PostResponse::parse_HTTP_body() {
	// If there's an expect header, check validity and ready body
	// before continuing
	if (requestHasHeader("expect")) {
		if (!send100Continue()) return response_status;
	}

	// If there's content-length, read it
	if (requestHasHeader("content-length")) readContentLength();

	// If it's chunked, get all the chunk
	// NOTE: In our server, chunked is only useful for CGI!
	else if (requestHasHeader("transfer-encoding") && requestIsCGI())
		readChunks();
	// If there's no content-length nor chunked, return error
	// No need to test for both existing at the same time: already tested on
	// HTTP header parser
	else
		response_status = BAD_REQUEST;

	return response_status;
}

bool PostResponse::send100Continue() {
	if (_request.header_fields.find("expect")->second != "100-continue") {
		response_status = BAD_REQUEST;
		return false;
	}

	if (!requestHasHeader("content-length") &&
		!requestHasHeader("transfer-encoding")) {
		response_status = BAD_REQUEST;
		return false;
	}

	if (requestHasHeader("content-length") &&
		stringToNumber<long>(
			_request.header_fields.find("content-length")->second) >
			_server.getClientMaxBodySize()) {
		response_status = PAYLOAD_TOO_LARGE;
		return false;
	}

	ssize_t bytesSent =
		send(_client_fd, "HTTP/1.1 100 Continue\r\n\r\n", 25, 0);
	if (bytesSent < 0) {
		perror("Error sending 100 Continue");  // TODO: Remove?
		response_status = INTERNAL_SERVER_ERROR;
		return false;
	}

	return true;
}

bool PostResponse::readBody() {
	struct epoll_event events[1];
	int nfds = epoll_wait(_epoll_fd, events, 1,
						  -1);	// Wait indefinitely for new data
	if (nfds < 0) {
		perror("Error in epoll_wait");	// TODO: Remove?
		response_status = INTERNAL_SERVER_ERROR;
		return false;
	}

	if (events[0].events & EPOLLIN) {
		while (true) {
			char newbuffer[8094] = {};

			ssize_t bytesRead =
				recv(_client_fd, newbuffer, sizeof(newbuffer), 0);

			if (bytesRead < 0) {
				if (errno == EAGAIN || errno == EWOULDBLOCK)
					// Non-blocking mode, no data available; exit the loop
					return true;
				else {
					response_status = INTERNAL_SERVER_ERROR;
					return false;
				}
			}

			_request.message_body.append(newbuffer, bytesRead);
		}
	}

	response_status =
		INTERNAL_SERVER_ERROR;	// TODO: Error if timeout? I guess?

	return false;
}

void PostResponse::readContentLength() {
	unsigned long content_length = stringToNumber<unsigned long>(
		_request.header_fields.find("content-length")->second);

	ssize_t bytes_to_read = content_length - _request.message_body.size();
	ssize_t total_bytes_read = 0;

	while (total_bytes_read < bytes_to_read) {
		char read_buffer[8096] = {};

		ssize_t buffer_size = std::min(bytes_to_read - total_bytes_read,
									   (ssize_t)sizeof(read_buffer));

		ssize_t bytes_read = recv(_client_fd, read_buffer, buffer_size, 0);

		if (bytes_read < 0) {
			// Handle error case (e.g., log the error, close the connection,
			// etc.)
			perror("Error reading from socket");
			break;	// Exit the loop on error
		}

		// Handle the case where the connection was closed prematurely
		if (bytes_read == 0) {
			// Client closed the connection
			break;	// Exit the loop if the connection is closed
		}

		_request.message_body.append(read_buffer, bytes_read);

		total_bytes_read += bytes_read;
	}
}

void PostResponse::readChunks() {
	removeFirstChunk();

	while (true) {
		ssize_t chunk_size = readChunkSizeFromSocket();

		if (chunk_size == 0) break;

		ssize_t total_bytes_read = 0;
		char read_buffer[8096] = {};

		while (total_bytes_read < chunk_size) {
			ssize_t read_size = std::min(chunk_size - total_bytes_read,
										 (ssize_t)sizeof(read_buffer));

			ssize_t bytes_read = recv(_client_fd, read_buffer, read_size, 0);
			if (bytes_read < 0) {
				// Handle error case (e.g., log the error, close the connection,
				// etc.)
				perror("Error reading from socket");
				break;	// Exit the loop on error
			}
			// Handle the case where the connection was closed prematurely
			if (bytes_read == 0) {
				// Client closed the connection
				return;	 // Exit the loop if the connection is closed
			}
			_request.message_body.append(read_buffer, bytes_read);
			total_bytes_read += bytes_read;
		}

		skipTrailingCRLF();
	}
}

// TODO: Remove first bit of chunk from message body in case it got read on
// run()
void PostResponse::removeFirstChunk() { ; }

ssize_t PostResponse::readChunkSizeFromSocket() {
	std::string chunk_size_str;
	char buffer;

	// Read byte by byte until we reach "\r\n" (end of chunk size line)
	while (true) {
		ssize_t bytes_read = recv(_client_fd, &buffer, 1, 0);
		if (bytes_read < 0) {
			// Handle error (e.g., log error, close connection, etc.)
			perror("Error reading chunk size");
			return -1;
		}
		if (bytes_read == 0) {
			// Connection closed by the client
			return 0;
		}

		chunk_size_str += buffer;

		if (chunk_size_str.length() >= 2 &&
			chunk_size_str[chunk_size_str.length() - 1] == '\n' &&
			chunk_size_str[chunk_size_str.length() - 2] == '\r') {
			chunk_size_str.resize(chunk_size_str.length() - 2);
			break;
		}
	}

	// Convert the chunk size from hex string to decimal
	ssize_t chunk_size = 0;
	chunk_size = std::strtol(chunk_size_str.c_str(), NULL, 16);

	// TODO: Handle errors

	return chunk_size;
}

void PostResponse::skipTrailingCRLF() {
	char crlf[2];
	ssize_t bytes_read = recv(_client_fd, crlf, 2, 0);

	if (bytes_read != 2 || crlf[0] != '\r' || crlf[1] != '\n')
		return;	 // TODO: Handle error?
}

std::string PostResponse::generateResponse() {
	unsigned short status = OK;
	setMatchLocationRoute();

	parse_HTTP_body();

	status = checkClientBodySize();
	if (status != OK) return loadErrorPage(status);

	status = checkBody();
	if (status != OK) return loadErrorPage(status);

	if (!requestIsCGI()) {
		status = extractFile();
		if (status != OK) return loadErrorPage(status);

		status = uploadFile();
		if (status != OK) return loadErrorPage(status);
	} else {
		// Send to CGI;
		std::cout << "!!GETPATH: " << getPath() << std::endl; // TESTE
    	std::string path = getPath();
    	CGI cgi(_request, _response, path);
    	cgi.handleCGIResponse();
    	loadCommonHeaders();
	}

  return getResponseStr();
}

bool PostResponse::requestHasHeader(const std::string &header) {
	if (_request.header_fields.find(header) != _request.header_fields.end())
		return true;

	return false;
}

bool PostResponse::requestIsCGI() {
	if (_request.uri.length() < 4) return false;

	std::string ending = _request.uri.substr(_request.uri.length() - 3);

	return (ending == ".py");
}

// Function to create directory
static bool createDirectory(const std::string &path) {
	if (mkdir(path.c_str(), 0777) == 0)
		return true;  // Directory created successfully
	else if (errno == EEXIST)
		return true;  // Directory already exists
	else
		return false;
}

short PostResponse::uploadFile() {
	extractFile();

	std::string directory = _server.getUpload(_locationRoute);
	if (directory.empty())
		return 500;	 // TODO: if upload_store empty, return error or have a
					 // default?
	if (directory.at(directory.length() - 1) != '/') directory += "/";
	if (!createDirectory(directory)) return FORBIDDEN;
	std::string target = directory + _file_to_upload.file_name;

	int file_fd =
		open(target.c_str(), O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
	if (file_fd == -1) return (FORBIDDEN);	// TODO: Adjust error

  size_t bytes_to_write = _file_to_upload.file_contents.size();

  if (write(file_fd, _file_to_upload.file_contents.c_str(), bytes_to_write) ==
      -1)
    return 500; // TODO: Adjust error, as well as binary octet stream

  if (close(file_fd) == -1)
    return (500); // TODO: Adjust error

  return 200;
}

short PostResponse::checkBody() {
	if (requestHasHeader("content-type") &&
		_request.header_fields.find("content-type")
				->second.find("multipart/") == 0) {
		_boundary = getBoundary();
		if (_boundary.empty()) return 400;

		_multipart_body = getMultipartBody(_boundary);
		if (_multipart_body.empty()) return 400;
	} else
		return 400;

	return 200;
}

// Returns boundary, returns empty if no boundary. Also checks content-type
// exists.
const std::string PostResponse::getBoundary() {
  std::multimap<std::string, std::string>::const_iterator
      content_type_header_field = _request.header_fields.find("content-type");
  if (content_type_header_field == _request.header_fields.end())
    return "";

  const std::string content_type = content_type_header_field->second;
  if (content_type.empty())
    return "";

  size_t boundary_position = content_type.find("boundary=");
  if (boundary_position == std::string::npos)
    return "";

  const std::string boundary = content_type.substr(boundary_position + 9);

  return boundary;
}

// Gets entire multipart as one contiguous vector
const std::vector<std::multimap<std::string, std::string> >
PostResponse::getMultipartBody(const std::string &boundary) {
  std::vector<std::multimap<std::string, std::string> > multipart_body;
  std::string full_boundary = "--" + boundary;
  std::string end_boundary = full_boundary + "--";

  size_t start_boundary_position = _request.message_body.find(full_boundary);
  if (start_boundary_position == std::string::npos)
    return multipart_body;

  while (start_boundary_position != std::string::npos) {
    start_boundary_position += full_boundary.length() + 2;
    size_t end_boundary_position =
        _request.message_body.find(full_boundary, start_boundary_position);

    if (_request.message_body.find(end_boundary, start_boundary_position) ==
        start_boundary_position)
      break;

    if (end_boundary_position == std::string::npos)
      return multipart_body;

    std::string subpart = _request.message_body.substr(
        start_boundary_position,
        end_boundary_position - start_boundary_position);

    std::multimap<std::string, std::string> submap = extractFields(subpart);
    if (submap.empty())
      break;
    multipart_body.push_back(submap);

    start_boundary_position = end_boundary_position;
  }

  return multipart_body;
}

// Gets submap for each part of the body
const std::multimap<std::string, std::string>
PostResponse::extractFields(const std::string &subpart) {
  std::multimap<std::string, std::string> submap;

  size_t header_end = subpart.find("\r\n\r\n");
  if (header_end == std::string::npos)
    return submap;

  std::string headers = subpart.substr(0, header_end + 2);
  std::string body = subpart.substr(header_end + 4);

  size_t start_separator = 0;
  size_t end_separator = headers.find("\r\n");

  while (end_separator != std::string::npos) {
    std::string field =
        headers.substr(start_separator, end_separator - start_separator);
    size_t colon_pos = field.find(":");
    if (colon_pos != std::string::npos) {
      std::string first = field.substr(0, colon_pos);
      std::string second = field.substr(colon_pos + 2);
      if (!first.empty() && !second.empty())
        submap.insert(std::make_pair(first, second));
    }

    start_separator = end_separator + 2;
    end_separator = headers.find("\r\n", start_separator);
  }

  submap.insert(
      std::make_pair("_File Contents", body.substr(0, body.length() - 2)));

  return submap;
}

short PostResponse::extractFile() {
  std::string content_disposition =
      _multipart_body[0].find("Content-Disposition")->second;

  _file_to_upload.name = extractFieldValue(content_disposition, "name");

  _file_to_upload.file_name =
      extractFieldValue(content_disposition, "filename");

  _file_to_upload.content_type =
      _multipart_body[0].find("Content-Type")->second;

  _file_to_upload.file_contents =
      _multipart_body[0].find("_File Contents")->second;

  return 200;
}

// Function to extract the value of a specified key (either "name" or
// "filename")
std::string PostResponse::extractFieldValue(const std::string &header,
                                            const std::string &field) {
  std::string key = field + "=";
  size_t start = header.find(key);

  if (start == std::string::npos)
    return "";

  start += key.length();

  bool inQuotes = false;
  if (header[start] == '"') {
    inQuotes = true;
    start++;
  }

  size_t end = start;
  while (end < header.length()) {
    if (inQuotes) {
      if (header[end] == '"') {
        inQuotes = false;
        break;
      }

    } else {
      if (header[end] == ';')
        break;
    }
    end++;
  }

  if (inQuotes && end < header.length() && header[end] == '"')
    end++;

  return header.substr(start, end - start);
}
