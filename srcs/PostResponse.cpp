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

	// Ver Client Body Buffer Size??
	//
	//

	this->upload_file();
	return "HI";
}

void PostResponse::upload_file() {
	const std::string boundary = get_boundary();

	const std::vector<std::multimap<std::string, std::string> > multipart_body =
		get_multipart_body(boundary);
}

// Returns boundary
const std::string PostResponse::get_boundary() {
	const std::string message_body = _request.message_body;
	const std::string content_type =
		_request.header_fields.find("content-type")->second;
	const std::string boundary =
		content_type.substr(content_type.find("boundary=") + 9);

	return boundary;
}

// Gets entire multipart as one contiguous vector
const std::vector<std::multimap<std::string, std::string> >
PostResponse::get_multipart_body(const std::string &boundary) {
	std::vector<std::multimap<std::string, std::string> > multipart_body;
	std::string full_boundary = "--" + boundary;
	std::string end_boundary = full_boundary + "--";

	size_t start_boundary_position = _request.message_body.find(full_boundary);
	while (start_boundary_position != std::string::npos) {
		start_boundary_position += full_boundary.length() + 2;
		size_t end_boundary_position =
			_request.message_body.find(full_boundary, start_boundary_position);

		if (_request.message_body.find(end_boundary, start_boundary_position) ==
			start_boundary_position)
			break;

		std::string subpart = _request.message_body.substr(
			start_boundary_position,
			end_boundary_position - start_boundary_position);

		std::multimap<std::string, std::string> submap =
			extract_fields(subpart);
		if (submap.empty()) break;
		multipart_body.push_back(submap);

		start_boundary_position = end_boundary_position;
	}

	return multipart_body;
}

// Gets submap for each part of the body
const std::multimap<std::string, std::string> PostResponse::extract_fields(
	const std::string &subpart) {
	std::multimap<std::string, std::string> submap;

	size_t header_end = subpart.find("\r\n\r\n");
	if (header_end == std::string::npos) return submap;

	std::string headers = subpart.substr(0, header_end);
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
			submap.insert(std::make_pair(first, second));
		}

		start_separator = end_separator + 2;
		end_separator = headers.find("\r\n", start_separator);
	}

	submap.insert(
		std::make_pair("Filebody", body.substr(0, body.length() - 2)));

	return submap;
}
