/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequestParser.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 16:12:57 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/11/02 11:28:29 by damachad         ###   ########.fr       */
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
	std::string decoded_uri;
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
	static unsigned short parseHTTPHeaders(const std::string& buffer_request,
										   HTTP_Request& HTTP);

   private:
	// Gets data HTTP_Request structure
	static bool addRequestLine(HTTP_Request& HTTP,
							   const std::string& first_line);
	static bool addHeaderFields(HTTP_Request& HTTP, const std::string& line);
	static void addMessageBody(HTTP_Request& HTTP, const std::string& line);
	static bool checkValidityOfHeaderFields(HTTP_Request& HTTP);
	static void extractQueries(HTTP_Request& HTTP);

	// Auxiliary functions for above functions
	static std::string trim(const std::string& str);
	static void trimNulls(std::string& s);
	static std::string decode(const std::string& encoded);
	static bool methodIsValid(const std::string& method);
	static bool urlIsValid(const std::string& url);
	static bool methodExists(const std::string& method);
	static bool protocolVersionIsValid(const ::std::string& protocol_version);

	// Private constructor and destructor: class is not instanciable
	HTTP_Request_Parser();
	~HTTP_Request_Parser();
};

std::ostream& operator<<(std::ostream& outstream, const HTTP_Request& request);

#endif

