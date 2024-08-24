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

#include <algorithm>

const HTTP_Request HTTP_Request_Parser::parse_HTTP_request(
	const std::string& request) {
	if (request.size() == 0) throw HTTPHeaderError("Empty Request");

	// Struct to return
	HTTP_Request HTTP;

	// Flags for parsing
	bool request_line_is_parsed = false;
	bool host_is_parsed = false;
	bool parsing_the_header = false;

	std::stringstream request_stream(request);
	std::string buffer;

	while (std::getline(request_stream, buffer)) {
		if (!request_line_is_parsed) {
			parse_request_line(HTTP, buffer);
			request_line_is_parsed = true;
		} else if (!host_is_parsed) {
			parse_host_line(HTTP, buffer);
			host_is_parsed = true;
		} else if (buffer != "\n\r" && parsing_the_header == false) {
			continue;
		} else {  // TODO: Still not correct!!
			parsing_the_header = true;
			add_message_body(HTTP, buffer);
		}
	}

	// std::cout << HTTP;

	return HTTP;
}

void HTTP_Request_Parser::parse_request_line(HTTP_Request& HTTP,
											 const std::string& first_line) {
	if (!whitespaces_are_valid(first_line, 2))
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
		!protocol_version_is_valid(protocol_version))
		throw HTTPHeaderError("Protocol version");

	HTTP.method = method;
	HTTP.uri = url;
	HTTP.protocol_version = protocol_version;
}

// Is validation necessary? Considering the host has already been connected
// to...
// Also, the IP address has already been validated on the server block
// Also, we're always using Firefox??
void HTTP_Request_Parser::parse_host_line(HTTP_Request& HTTP,
										  const std::string& host) {
	if (!whitespaces_are_valid(host, 1))
		throw HTTPHeaderError("Invalid Whitespaces");

	std::stringstream line_stream(host);

	std::string address;
	line_stream >> address;
	if (address != "Host:") throw HTTPHeaderError("Host");

	address.clear();
	line_stream >> address;
	if (address.size() == 0) throw HTTPHeaderError("Address");

	HTTP.host = address;
}

// Ensures correct number of whitespaces (two ' ' in the request line, 1 in Host
// line, etc) Indirectly sets up the next functions
bool HTTP_Request_Parser::whitespaces_are_valid(const std::string& first_line,
												unsigned int limit) {
	unsigned int whitespace_count = 0;
	unsigned int space_count = 0;

	for (std::string::const_iterator it = first_line.begin();
		 it != first_line.end(); it++) {
		if (std::isspace(*it)) whitespace_count++;
		if (*it == ' ') space_count++;
	}

	if (space_count != limit || whitespace_count != limit + 1) return false;

	return true;
}

bool HTTP_Request_Parser::method_is_valid(const std::string& method) {
	if (method == "GET" || method == "POST" || method == "DELETE") return true;
	return false;
}

bool HTTP_Request_Parser::url_is_valid(const std::string& url) {
	// Target should always start with a /
	if (url[0] != '/') return false;

	// There should be no gragmenet urls
	if (url.find("#") != std::string::npos) return false;

	// There should be only one ?
	if (std::count(url.begin(), url.end(), '?') > 1) return false;

	// If there's a ?, the number of key-value pairs (std::count of '=') should
	// be equal to std::count of '&' - 1
	if (url.find('?') != std::string::npos &&
		(std::count(url.begin(), url.end(), '=') !=
		 std::count(url.begin(), url.end(), '&') + 1))
		return false;

	return true;
}

bool HTTP_Request_Parser::protocol_version_is_valid(
	const std::string& protocol_version) {
	if (protocol_version == "HTTP/1.1" || protocol_version == "HTTP/1.0" ||
		protocol_version == "HTTP/0.9")
		return true;
	return false;
}

void HTTP_Request_Parser::add_message_body(HTTP_Request& HTTP,
										   const std::string& line) {
	HTTP.message_body += line;
}

std::ostream& operator<<(std::ostream& outstream, const HTTP_Request& request) {
	outstream << "HTTP Request: \n"
			  << "Method: " << request.method << "\n"
			  << "URI: " << request.uri << "\n"
			  << request.protocol_version << "\n"
			  << "Message body: " << request.message_body << std::endl;

	return outstream;
}
