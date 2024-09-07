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
	HTTP_Request HTTP;

	// Flags for parsing
	bool request_line_is_parsed = false;
	bool header_is_parsed = false;

	std::istringstream request_stream(request);
	std::string buffer;

	while (std::getline(request_stream, buffer)) {
		if (!request_line_is_parsed) {
			add_req_line(HTTP, buffer);
			request_line_is_parsed = true;
		} else if (buffer != "\r" && !header_is_parsed)
			add_header_fields(HTTP, buffer);
		else if (buffer == "\r " || buffer.empty()) {
			header_is_parsed = true;
		} else
			add_message_body(HTTP, buffer);
	}

	extract_queries(HTTP);

	std::cout << "***Extracted HTTP***\n\n" << HTTP;

	return HTTP;
}

void HTTP_Request_Parser::add_req_line(HTTP_Request& HTTP,
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

	HTTP.method = stringToMethod(method);
	HTTP.uri = url;
	HTTP.protocol_version = protocol_version;
}

void HTTP_Request_Parser::add_header_fields(HTTP_Request& HTTP,
											const std::string& line) {
	size_t colon_pos = line.find(':');

	// Validate the presence of colon
	if (colon_pos == std::string::npos) throw HTTPHeaderError("HeaderField");

	// Extract key and value
	std::string key = line.substr(0, colon_pos);
	std::string comma_separated_values = line.substr(colon_pos + 1);

	// Trim whitespace from key and value
	key.erase(key.find_last_not_of(" \n\r\t") + 1);
	comma_separated_values.erase(
		0, comma_separated_values.find_first_not_of(" \n\r\t"));

	// Validate the key and value (if needed)
	if (key.empty() || comma_separated_values.empty())
		throw HTTPHeaderError("HeaderField");

	// Add comma_separated_values into a set
	std::stringstream ss(comma_separated_values);
	std::string value;

	while (std::getline(ss, value, ',')) {
		value.erase(value.find_last_not_of(" \n\r\t") + 1);
		value.erase(0, value.find_first_not_of(" \n\r\t"));

		if (!value.empty())	 // Ignore empty values
			HTTP.header_fields.insert(
				std::pair<std::string, std::string>(key, value));
	}
}

// Fields without validation (NOTE:is it needed?)
void HTTP_Request_Parser::add_message_body(HTTP_Request& HTTP,
										   const std::string& line) {
	HTTP.message_body += line;
}

// Extracts queries from URI to HTTP struct. Also updates URI.
void HTTP_Request_Parser::extract_queries(HTTP_Request& HTTP) {
	size_t delimiter_pos = HTTP.uri.find('?');

	if (delimiter_pos == std::string::npos) return;	 // Returns if no ? found

	std::string queries = HTTP.uri.substr(delimiter_pos + 1);  // Finds queries
	HTTP.uri = HTTP.uri.substr(0, delimiter_pos);  // Takes our query from URI

	// Extract queries
	std::stringstream ss(queries);
	std::string pair;

	while (std::getline(ss, pair, '&')) {
		size_t pair_delimiter_pos = pair.find('=');
		std::string key;
		std::string value;
		if (pair_delimiter_pos != std::string::npos) {
			key = pair.substr(0, pair_delimiter_pos);
			value = pair.substr(pair_delimiter_pos + 1);
		} else
			key = pair;	 // If no '=' was found, key has empty value

		trim(key);
		trim(pair);

		if (!key.empty())
			HTTP.query_fields.insert(
				std::pair<std::string, std::string>(key, value));
	}
	return;
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

std::string HTTP_Request_Parser::trim(const std::string& str) {
	size_t start = str.find_first_not_of(" \n\r\t");
	if (start == std::string::npos) return "";

	size_t end = str.find_last_not_of(" \n\r\t");
	return str.substr(start, end - start + 1);
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

std::ostream& operator<<(std::ostream& outstream, const HTTP_Request& request) {
	outstream << "HTTP Request: \n"
			  << "Method: " << request.method << "\n"
			  << "URI: " << request.uri << "\n"
			  << "Protocol: " << request.protocol_version << "\n";

	outstream << "Header fields: \n";
	for (std::multimap<std::string, std::string>::const_iterator it =
			 request.header_fields.begin();
		 it != request.header_fields.end(); it++)
		outstream << "\t" << it->first << ": " << it->second << "\n";

	outstream << "Query fields: \n";
	for (std::multimap<std::string, std::string>::const_iterator it =
			 request.query_fields.begin();
		 it != request.query_fields.end(); it++)
		outstream << "\t" << it->first << ": " << it->second << "\n";

	outstream << "Message body: " << request.message_body << std::endl;

	return outstream;
}
