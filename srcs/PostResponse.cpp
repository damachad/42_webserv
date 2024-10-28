/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 13:21:15 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/10/25 11:36:25 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "PostResponse.hpp"

#include <sys/stat.h>

#include <sstream>

#include "CGI.hpp"
#include "Helpers.hpp"
#include "Webserv.hpp"

static unsigned short response_status = OK;

PostResponse::PostResponse(const Server &server, HTTP_Request &request,
						   int client_fd, int epoll_fd)
	: AResponse(server, request), _client_fd(client_fd), _epoll_fd(epoll_fd) {}

PostResponse::PostResponse(const PostResponse &src) : AResponse(src) {}

PostResponse::~PostResponse() {}

unsigned short PostResponse::parseHTTPBody() {
	// If there's an expect header, check validity and ready body
	// before continuing
	if (requestHasHeader("expect")) {
		if (!send100Continue()) return response_status;
	}

	// If there's content-length, read it
	if (requestHasHeader("content-length")) readContentLength();

	// If it's chunked, get all the chunk
	// NOTE: In our server, chunked is only useful for CGI!?
	else if (requestHasHeader("transfer-encoding") && isCGI())
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
		response_status = INTERNAL_SERVER_ERROR;
		return false;
	}

	return true;
}

// NOTE: Remove??? Function not used?
//
/* bool PostResponse::readBody() {
	struct epoll_event events[1];
	int nfds = epoll_wait(_epoll_fd, events, 1,
						  -1);	// Wait indefinitely for new data
	if (nfds < 0) {
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
			} else if (bytesRead == 0)
				return true;

			_request.message_body.append(newbuffer, bytesRead);
		}
	}

	response_status = INTERNAL_SERVER_ERROR;

	return false;
} */

void PostResponse::readContentLength() {
	unsigned long content_length = stringToNumber<unsigned long>(
		_request.header_fields.find("content-length")->second);

	if (content_length + total_used_storage > MAX_STORAGE_SIZE) {
		response_status = INTERNAL_SERVER_ERROR;
		return;
	}

	ssize_t bytes_to_read = content_length - _request.message_body.size() - 1;
	ssize_t total_bytes_read = 0;

	while (total_bytes_read < bytes_to_read) {
		char read_buffer[8096] = {};

		ssize_t buffer_size = std::min(bytes_to_read - total_bytes_read,
									   (ssize_t)sizeof(read_buffer));

		ssize_t bytes_read = recv(_client_fd, read_buffer, buffer_size, 0);

		if (bytes_read < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
			response_status = INTERNAL_SERVER_ERROR;
			return;	 // Exit the loop on error
		}

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

		if (chunk_size == -1) {
			response_status = BAD_REQUEST;
			return;
		}

		if (chunk_size == 0) break;

		ssize_t total_bytes_read = 0;
		char read_buffer[8096] = {};

		while (total_bytes_read < chunk_size) {
			if (total_used_storage + _request.message_body.size() >
				MAX_STORAGE_SIZE) {
				response_status = INTERNAL_SERVER_ERROR;
				return;
			}

			ssize_t read_size = std::min(chunk_size - total_bytes_read,
										 (ssize_t)sizeof(read_buffer));

			ssize_t bytes_read = recv(_client_fd, read_buffer, read_size, 0);
			if (bytes_read < 0) {
				// Handle error case (e.g., log the error, close the connection,
				// etc.)
				response_status = INTERNAL_SERVER_ERROR;
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

		if (skipTrailingCRLF() == -1) {
			response_status = BAD_REQUEST;
			return;
		}
	}
}

// TODO: Remove first bit of chunk from message body in case it got read on
// run()
// NOTE: (from future Miguel) Why???
void PostResponse::removeFirstChunk() { ; }

ssize_t PostResponse::readChunkSizeFromSocket() {
	std::string chunk_size_str;
	char buffer;

	// Read byte by byte until we reach "\r\n" (end of chunk size line)
	while (true) {
		ssize_t bytes_read = recv(_client_fd, &buffer, 1, 0);
		if (bytes_read < 0) return -1;
		if (bytes_read == 0) return 0;

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

	return chunk_size;
}

int PostResponse::skipTrailingCRLF() {
	char crlf[2];
	ssize_t bytes_read = recv(_client_fd, crlf, 2, 0);

	if (bytes_read != 2 || crlf[0] != '\r' || crlf[1] != '\n') return -1;

	return 0;
}

static std::string generateDefaultUploadResponse() {
	return "<!DOCTYPE html>\n"
			"<html lang=\"en\">\n"
			"<head>\n"
			"\t<meta charset=\"UTF-8\">\n"
			"\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
			"\t<link rel=\"icon\" href=\"assets/favicon.ico\" type=\"image/x-icon\">\n"
    		"\t<link rel=\"stylesheet\" href=\"assets/css/style.css\">\n"
			"\t<title>Upload Successful</title>\n"
			"</head>\n"
			"<body>\n"
			"\t<h1>File Uploaded Successfully!</h1>\n"
			"\t<p>Your file has been uploaded.</p>\n"
			"\t<a href=\"index.html\">Back to Index</a>\n"
			"</body>\n"
			"</html>\n";
}

std::string PostResponse::generateResponse() {
	unsigned short status = OK;
	setMatchLocationRoute();

	if ((status = parseHTTPBody()) != OK) return loadErrorPage(status);
	
	if ((status = checkClientBodySize()) != OK) return loadErrorPage(status);

	if (!isCGI()) {

		if ((status = checkFormData()) != OK) return loadErrorPage(status);

		status = checkBody();
		if (status != OK) return loadErrorPage(status);
		
		status = extractFile();
		if (status != OK) return loadErrorPage(status);

		if ((status = uploadFile()) != OK) return loadErrorPage(status);
		_response.body = generateDefaultUploadResponse();
		_response.status = CREATED;
	} else {
		// Send to CGI;
		std::string path = getPath();
		
		CGI cgi(_request, _response, path);
		cgi.handleCGIResponse();
		if (_response.status != 200) loadErrorPage(_response.status);
	}
	loadCommonHeaders();

	return getResponseStr();
}

bool PostResponse::requestHasHeader(const std::string &header) {
	if (_request.header_fields.find(header) != _request.header_fields.end())
		return true;

	return false;
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

// Checks if a file with that name exists and returns its size
static long fileExistsSize(std::string &target) {
	struct stat fileInfo;
	
	if (stat(target.c_str(), &fileInfo) == 0)
		return fileInfo.st_size;
	return 0;
}

short PostResponse::uploadFile() {
	short status = extractFile();
	if (status != OK) return status;

	std::string directory = _server.getUpload(_locationRoute);
	if (directory.empty())
		directory = assemblePath(_server.getRoot(_locationRoute),
								 "default_upload_directory");
	if (directory.at(directory.length() - 1) != '/') directory += "/";
	if (!createDirectory(directory)) return FORBIDDEN;
	std::string target = directory + _file_to_upload.file_name;
	long existingFileSize = fileExistsSize(target);
	
	int file_fd =
		open(target.c_str(), O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
	if (file_fd == -1) return FORBIDDEN;

	size_t bytes_to_write = _file_to_upload.file_contents.size();

	if (write(file_fd, _file_to_upload.file_contents.c_str(), bytes_to_write) ==
		-1)
		return FORBIDDEN;

	if (close(file_fd) == -1) return INTERNAL_SERVER_ERROR;

	total_used_storage += fileExistsSize(target) - existingFileSize;
	
	return OK;
}

short PostResponse::checkBody() {
	if (requestHasHeader("content-type") &&
		_request.header_fields.find("content-type")
				->second.find("multipart/") == 0) {
		_boundary = getBoundary();
		if (_boundary.empty()) return BAD_REQUEST;
		_multipart_body = getMultipartBody(_boundary);
		if (_multipart_body.empty()) return BAD_REQUEST;
	} else {
		return 400;
	}
	return OK;
}

short PostResponse::checkFormData() {
	std::map<std::string, std::string>::iterator it =
		_request.header_fields.find("content-type");

	if (it == _request.header_fields.end()) return BAD_REQUEST;

	if (it->second.find("multipart/form-data") == std::string::npos)
		return UNSUPPORTED_MEDIA_TYPE;

	return OK;
}

// Returns boundary, returns empty if no boundary. Also checks content-type
// exists.
const std::string PostResponse::getBoundary() {
	std::multimap<std::string, std::string>::const_iterator
		content_type_header_field = _request.header_fields.find("content-type");
	if (content_type_header_field == _request.header_fields.end()) return "";

	const std::string content_type = content_type_header_field->second;
	if (content_type.empty()) return "";

	size_t boundary_position = content_type.find("boundary=");
	if (boundary_position == std::string::npos) return "";

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
	if (start_boundary_position == std::string::npos) return multipart_body;

	while (start_boundary_position != std::string::npos) {
		start_boundary_position += full_boundary.length() + 2;
		size_t end_boundary_position =
			_request.message_body.find(full_boundary, start_boundary_position);

		if (_request.message_body.find(end_boundary, start_boundary_position) ==
			start_boundary_position)
			break;

		if (end_boundary_position == std::string::npos) return multipart_body;

		std::string subpart = _request.message_body.substr(
			start_boundary_position,
			end_boundary_position - start_boundary_position);

		std::multimap<std::string, std::string> submap = extractFields(subpart);
		if (submap.empty()) break;
		multipart_body.push_back(submap);

		start_boundary_position = end_boundary_position;
	}

	return multipart_body;
}

// Gets submap for each part of the body
const std::multimap<std::string, std::string> PostResponse::extractFields(
	const std::string &subpart) {
	std::multimap<std::string, std::string> submap;

	size_t header_end = subpart.find("\r\n\r\n");
	if (header_end == std::string::npos) return submap;

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

	if (_multipart_body.empty())
		return 500;

    std::multimap<std::string, std::string>::iterator content_disposition_it = 
		_multipart_body[0].find("Content-Disposition");
    if (content_disposition_it == _multipart_body[0].end())
        return 500;
    std::string content_disposition = content_disposition_it->second;

    _file_to_upload.name = extractFieldValue(content_disposition, "name");
    _file_to_upload.file_name = 
		extractFieldValue(content_disposition, "filename");

    std::multimap<std::string, std::string>::iterator content_type_it = 
		_multipart_body[0].find("Content-Type");
    if (content_type_it == _multipart_body[0].end())
        return 500;
    _file_to_upload.content_type = content_type_it->second;

    std::multimap<std::string, std::string>::iterator file_contents_it = 
		_multipart_body[0].find("_File Contents");
    if (file_contents_it == _multipart_body[0].end())
        return 500;
    _file_to_upload.file_contents = file_contents_it->second;

    return 200;
}

// Function to extract the value of a specified key (either "name" or
// "filename")
std::string PostResponse::extractFieldValue(const std::string &header,
											const std::string &field) {
	std::string key = field + "=";
	size_t start = header.find(key);

	if (start == std::string::npos) return "";

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
			if (header[end] == ';') break;
		}
		end++;
	}

	if (inQuotes && end < header.length() && header[end] == '"') end++;

	return header.substr(start, end - start);
}
