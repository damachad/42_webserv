/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequestParser.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 16:12:47 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/09/30 11:46:51 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTPRequestParser.hpp"

#include "Helpers.hpp"
#include "Webserv.hpp"

static int response_status = OK;

unsigned short HTTP_Request_Parser::parse_HTTP_headers(
	const std::string& buffer_request, HTTP_Request& HTTP) {
	if (buffer_request.empty() ||
		buffer_request.find_first_not_of(" \r\n\t") == std::string::npos) {
		response_status = BAD_REQUEST;
		return response_status;
	}

	// Buffers for parsing
	std::stringstream buffer_stream(buffer_request);
	std::string buffer;

	// Flags for parsing
	bool request_line_is_parsed = false;
	bool header_is_parsed = false;

	while (std::getline(buffer_stream, buffer)) {
		// Parses request line
		if (!request_line_is_parsed) {
			if (!add_req_line(HTTP, buffer)) return response_status;
			request_line_is_parsed = true;
		}  // Parses header fields
		else if (buffer != "\r" && !header_is_parsed) {
			if (!add_header_fields(HTTP, buffer)) return response_status;
		}  // Notes end of header fields
		else if (buffer == "\r" || buffer.empty()) {
			header_is_parsed = true;
			break;
		}
	}

	extract_queries(HTTP);

	std::string remaining_body((std::istreambuf_iterator<char>(buffer_stream)),
							   std::istreambuf_iterator<char>());
	HTTP.message_body =
		remaining_body;	 // Append the remaining body, if such exists

	return response_status;
}

bool HTTP_Request_Parser::add_req_line(HTTP_Request& HTTP,
									   const std::string& first_line) {
	if (std::isspace(first_line[0])) return false;

	std::stringstream line_stream(first_line);

	std::string method;
	line_stream >> method;
	if (method.size() == 0 || !method_is_valid(method)) {
		response_status = METHOD_NOT_ALLOWED;
		return false;
	}

	std::string url;
	line_stream >> url;

	std::string decoded_uri = decode(url);
	if (decoded_uri.size() == 0 || !url_is_valid(decoded_uri)) {
		response_status = BAD_REQUEST;
		return false;
	}

	if (decoded_uri.size() > URL_MAX_SIZE) {
		response_status = URI_TOO_LONG;
		return false;
	}

	std::string protocol_version;
	line_stream >> protocol_version;
	if (protocol_version.size() == 0 ||
		!protocol_version_is_valid(protocol_version)) {
		response_status = HTTP_VERSION_NOT_SUPPORTED;
		return false;
	}

	HTTP.method = stringToMethod(method);
	HTTP.uri = trim(url);
	HTTP.decoded_uri = decoded_uri;
	HTTP.protocol_version = trim(protocol_version);

	return true;
}

bool HTTP_Request_Parser::add_header_fields(HTTP_Request& HTTP,
											const std::string& line) {
	size_t colon_pos = line.find(':');

	// Validate the presence of colon
	if (colon_pos == std::string::npos) {
		response_status = BAD_REQUEST;
		return false;
	}

	// Extract key and value
	std::string key = line.substr(0, colon_pos);
	std::string comma_separated_values = line.substr(colon_pos + 1);

	// Trim whitespace from key and value
	key.erase(key.find_last_not_of(" \n\r\t") + 1);
	comma_separated_values.erase(
		0, comma_separated_values.find_first_not_of(" \n\r\t"));

	// Validate the key and value (if needed)
	if (key.empty() || comma_separated_values.empty()) {
		response_status = BAD_REQUEST;
		return false;
	}

	// Normalize key to lowercase
	key = toLower(key);

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

	return true;
}

// Fields without validation
void HTTP_Request_Parser::add_message_body(HTTP_Request& HTTP,
										   const std::string& line) {
	if (!line.empty()) {
		HTTP.message_body += line;
		HTTP.message_body += "\r\n";
	}
}

// Checks validity of HTTP header fields
bool HTTP_Request_Parser::check_validity_of_header_fields(HTTP_Request& HTTP) {
	// NOTE: User-Agent is not a strictly mandatory field, but helps for
	// evaluation
	if (HTTP.header_fields.count("user-agent") != 1) {
		response_status = BAD_REQUEST;
		return false;
	}

	std::string user_agent = HTTP.header_fields.find("user-agent")->second;

	bool has_curl = user_agent.find("curl") != std::string::npos;
	bool has_mozilla = user_agent.find("Mozilla") != std::string::npos;
	bool has_firefox = user_agent.find("Firefox") != std::string::npos;
	bool has_siege = user_agent.find("siege") != std::string::npos;

	// Checks that the request is from Curl, Mozilla Firefox or Mozilla
	// Siege
	if (!(has_curl || (has_mozilla && (has_firefox || has_siege)))) {
		response_status = BAD_REQUEST;
		return false;
	}

	// NOTE: Host is mandatory on HTTP 1.1
	if ((HTTP.header_fields.count("host") != 1 &&
		 HTTP.protocol_version == "HTTP/1.1") ||
		HTTP.header_fields.count("host") > 1) {
		response_status = BAD_REQUEST;
		return false;
	}

	// Only one of content-length or transfer-encoding is allowed
	if (HTTP.header_fields.count("content-length") +
			HTTP.header_fields.count("transfer-encoding") >
		1) {
		response_status = BAD_REQUEST;
		return false;
	}

	// Checks if content-length is bigger than body, or if a body exists
	// with no content length nor transfer-encoding
	if (HTTP.header_fields.find("content-length") != HTTP.header_fields.end()) {
		unsigned long content_length = stringToNumber<unsigned long>(
			HTTP.header_fields.find("content-length")->second);
		if (content_length != HTTP.message_body.size()) {
			response_status = BAD_REQUEST;
			return false;
		}
	} else	// If there is no content-length
	{
		if (!HTTP.message_body.empty() &&
			HTTP.header_fields.find("transfer-encoding") !=
				HTTP.header_fields.end())  // If there is a body and no
										   // transfer-encoding either
		{
			response_status = LENGTH_REQUIRED;
			return false;
		}
	}

	return true;
}

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

std::string HTTP_Request_Parser::trim(const std::string& str) {
	size_t start = str.find_first_not_of(" \n\r\t");
	if (start == std::string::npos) return "";

	size_t end = str.find_last_not_of(" \n\r\t");
	return str.substr(start, end - start + 1);
}

// Function to trim null characters ('\0') from a string
void HTTP_Request_Parser::trimNulls(std::string& s) {
	size_t pos = s.find_last_not_of('\0');

	if (pos == std::string::npos) {
		s.clear();
	} else {
		s.resize(pos + 1);
	}
}

std::string HTTP_Request_Parser::decode(const std::string& encoded) {
	std::ostringstream decoded;
	for (size_t i = 0; i < encoded.length(); ++i) {
		if (encoded[i] == '%' && i + 2 < encoded.length() &&
			std::isxdigit(encoded[i + 1]) && std::isxdigit(encoded[i + 2])) {
			int value;
			std::istringstream ss(encoded.substr(i + 1, 2));
			ss >> std::hex >> value;
			decoded << static_cast<char>(value);
			i += 2;
		} else {
			decoded << encoded[i];
		}
	}
	return decoded.str();
}

bool HTTP_Request_Parser::method_is_valid(const std::string& method) {
	if (method == "GET" || method == "POST" || method == "DELETE") return true;
	return false;
}

bool HTTP_Request_Parser::url_is_valid(const std::string& url) {
	// Target should always start with a /
	if (url[0] != '/') return false;

	// There should be no fragmenet urls
	if (url.find("#") != std::string::npos) return false;

	// There should be only one ?
	if (std::count(url.begin(), url.end(), '?') > 1) return false;

	// If there's a ?, the number of key-value pairs (std::count of '=')
	// should be equal to std::count of '&' - 1
	if (url.find('?') != std::string::npos &&
		(std::count(url.begin(), url.end(), '=') !=
		 std::count(url.begin(), url.end(), '&') + 1))
		return false;

	// URL_MAX_SIZE should not be bigger than default value
	if (url.size() > URL_MAX_SIZE) return false;

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
