/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequestParser.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 16:12:57 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/09/17 14:18:03 by mde-sa--         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_REQUEST_PARSER_HPP
#define HTTP_REQUEST_PARSER_HPP

#include "Exceptions.hpp"
#include "Helpers.hpp"
#include "Webserv.hpp"

struct HTTP_Request {
	// Request Line
	enum Method method;
	std::string uri;
	std::string protocol_version;

	// Header
	// NOTE: A multimap organizes key-value pairs, while allowing for repeat
	// keys. Important for some cookies, and perhaps request validation???
	std::multimap<std::string, std::string> header_fields;

	// Query Parameters
	std::multimap<std::string, std::string> query_fields;

	// Request Body
	std::string message_body;

	// NOTE: Default constructor to initialize the members
	HTTP_Request() : method(UNKNOWN) {};
};

class HTTP_Request_Parser {
   public:
	// Main function, parses request
	static unsigned short parse_HTTP_request(const std::string& buffer_request,
											 HTTP_Request& HTTP);

   private:
	// Gets data HTTP_Request structure
	static bool add_req_line(HTTP_Request& HTTP, const std::string& first_line);
	static bool add_header_fields(HTTP_Request& HTTP, const std::string& line);
	static void add_message_body(HTTP_Request& HTTP, const std::string& line);
	static bool check_validity_of_header_fields(HTTP_Request& HTTP);
	static void extract_queries(HTTP_Request& HTTP);

	// Auxiliary functions for above functions
	static std::string trim(const std::string& str);
	static std::string decode(const std::string& encoded);
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

// Curl:
//
// HTTP Request:
// Method: 1
// URI: /ping
// Protocol: HTTP/1.1
// Header fields:
//        Accept: */*
//        Host: localhost:9999
//        User-Agent: curl/7.81.0
// Query fields:
//        tia: gorbachov
//        user: miguel
// Message body:
//
// Firefox:
// Header fields:
//        Accept: text/html
//        Accept: application/xhtml+xml
//        Accept: application/xml;q=0.9
//        Accept: image/avif
//        Accept: image/webp
//        Accept: image/png
//        Accept: image/svg+xml
//        Accept: */*;q=0.8
//        Accept-Encoding: gzip
//        Accept-Encoding: deflate
//        Accept-Encoding: br
//        Accept-Encoding: zstd
//        Accept-Language: en-US
//        Accept-Language: en;q=0.5
//        Connection: keep-alive
//        Host: localhost:9999
//        Priority: u=0
//        Priority: i
//        Sec-Fetch-Dest: document
//        Sec-Fetch-Mode: navigate
//        Sec-Fetch-Site: none
//        Sec-Fetch-User: ?1
//        Upgrade-Insecure-Requests: 1
//        User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:130.0) Gecko/20100101
//        Firefox/130.0
