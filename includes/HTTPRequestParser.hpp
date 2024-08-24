/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTP_request_parser.hpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mde-sa-- <mde-sa--@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 16:12:57 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/08/21 16:12:58 by mde-sa--         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_REQUEST_PARSER
#define HTTP_REQUEST_PARSER

#include "Webserv.hpp"

typedef struct HTTP_Request {
	// Request Line
	std::string method;
	std::string uri;
	std::string protocol_version;

	// Request Header
	std::string host;

	// Request Body
	std::string message_body;
} HTTP_Request;

class HTTP_Request_Parser {
   public:
	// Main function, parses request
	static const HTTP_Request parse_HTTP_request(const std::string& request);

   private:
	// Gets data to request_line
	static void parse_request_line(HTTP_Request& HTTP,
								   const std::string& first_line);

	static void parse_host_line(HTTP_Request& HTTP, const std::string& host);
	static void add_message_body(HTTP_Request& HTTP, const std::string& line);

	// Self-explanatory bools to check HTTP Requests' request line
	static bool whitespaces_are_valid(const std::string& first_line,
									  unsigned int limit);
	static bool method_is_valid(const std::string& method);
	static bool url_is_valid(const std::string& url);
	static bool protocol_version_is_valid(
		const ::std::string& protocol_version);

	// Private constructor and destructor: class is not instanciable
	HTTP_Request_Parser();
	~HTTP_Request_Parser();
};

std::ostream& operator<<(std::ostream& outstream, const HTTP_Request& request);

#endif

// Firefox:
// GET / HTTP/1.1
// Host: 127.0.0.1:8082
// User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:128.0) Gecko/20100101
// Firefox/128.0 Accept:
// text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/png,image/svg+xml,*/*;q=0.8
// Accept-Language: en-US,en;q=0.5
// Accept-Encoding: gzip, deflate, br, zstd
// Connection: keep-alive
// Upgrade-Insecure-Requests: 1
// Sec-Fetch-Dest: document
// Sec-Fetch-Mode: navigate
// Sec-Fetch-Site: none
// Sec-Fetch-User: ?1
// Priority: u=0, i
//
