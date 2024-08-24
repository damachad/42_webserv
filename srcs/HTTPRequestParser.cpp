/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTP_request_parser.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mde-sa-- <mde-sa--@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 16:12:47 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/08/21 16:13:11 by mde-sa--         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPRequestParser.hpp"

const HTTP_Request HTTP_Request_Parser::parse_HTTP_request(
	const std::string& request) {
	if (request.size() == 0) throw HTTPHeaderError("Empty Request");

	// Struct to return
	struct HTTP_Request HTTP;

	// Flags for parsing
	bool http_is_valid = false;
	bool request_line_is_parsed = false;
	bool header = false;

	std::stringstream request_stream(request);
	std::string buffer;

	while (!http_is_valid) {
		std::getline(request_stream, buffer);
		if (!request_line_is_parsed) {
			parse_request_line(HTTP, buffer);
			request_line_is_parsed = true;
		} else {
			(void)header;
			header = true;
		}
	}

	return HTTP;
}

void HTTP_Request_Parser::parse_request_line(HTTP_Request& HTTP,
											 const std::string& first_line) {
	if (!whitespaces_are_valid(first_line))
		throw HTTPHeaderError("Invalid Whitespaces");

	std::stringstream line_stream(first_line);

	std::string method;
	line_stream >> method;
	if (method.size() == 0 || !method_is_valid(method))
		throw HTTPHeaderError("Method");

	std::string url;
	line_stream >> url;
	if (url.size() == 0 || !url_is_valid(url)) throw HTTPHeaderError("URI");

	std::string protocol_version;
	line_stream >> protocol_version;
	if (protocol_version.size() == 0 ||
		protocol_version_is_valid(protocol_version))
		throw HTTPHeaderError("Protocol version");

	HTTP.method = method;
	HTTP.uri = url;
	HTTP.protocol_version = protocol_version;
}

// Ensures correct number of whitespaces (two ' ' in the request line)
// Indirectly sets up the next three functions
bool HTTP_Request_Parser::whitespaces_are_valid(const std::string& first_line) {
	int whitespace_count = 0;
	int space_count = 0;

	for (std::string::const_iterator it = first_line.begin();
		 it != first_line.end(); it++) {
		if (std::isspace(*it)) whitespace_count++;
		if (*it == ' ') space_count++;
	}

	if (space_count != 2 || whitespace_count != 2) return false;

	return true;
}

bool HTTP_Request_Parser::method_is_valid(const std::string& method) {
	if (method == "GET" || method == "POST" || method == "DELETE") return true;
	return false;
}

// TODO:Gotta work on URL parsing now
bool HTTP_Request_Parser::url_is_valid(const std::string& url) {
	(void)url;
	return true;
}

bool HTTP_Request_Parser::protocol_version_is_valid(
	const std::string& protocol_version) {
	if (protocol_version == "HTTP/1.1" || protocol_version == "HTTP/1.0" ||
		protocol_version == "HTTP/0.9")
		return true;
	return false;
}
