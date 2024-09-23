/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mde-sa-- <mde-sa--@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 13:21:15 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/09/14 10:41:19 by mde-sa--         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "PostResponse.hpp"

#include <sys/stat.h>

PostResponse::PostResponse(const Server &server, const HTTP_Request &request)
	: AResponse(server, request) {}

PostResponse::PostResponse(const PostResponse &src) : AResponse(src) {}

PostResponse::~PostResponse() {}

std::string PostResponse::generateResponse() {
	setMatchLocationRoute();
	unsigned short status;

	status = checkSize();
	if (status != 200) return loadErrorPage(status);

	status = checkClientBodySize();
	if (status != 200) return loadErrorPage(status);

	status = checkMethod();
	if (status != 200) return loadErrorPage(status);

	status = checkBody();
	if (status != 200) return loadErrorPage(status);

	status = uploadFile();
	if (status != 200) return loadErrorPage(status);
	// Ver Client Body Buffer Size??
	//
	//
	return "HI";
}

short PostResponse::uploadFile() {
	extractFile();

	std::string directory = "file_uploads/";
	std::string target = directory + _file_to_upload.file_name;

	int file_fd =
		open(target.c_str(), O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
	if (file_fd == -1) return (500);  // TODO: Adjust error

	size_t bytes_to_write = _file_to_upload.file_contents.size();

	if (write(file_fd, _file_to_upload.file_contents.c_str(), bytes_to_write) ==
		-1)
		return 500;	 // TODO: Adjust error, as well as binary octet stream

	if (close(file_fd) == -1) return (500);	 // TODO: Adjust error

	return 200;
}

short PostResponse::checkBody() {
	_boundary = getBoundary();
	if (_boundary.empty()) return 400;

	_multipart_body = getMultipartBody(_boundary);
	if (_multipart_body.empty()) return 400;

	return 200;
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
