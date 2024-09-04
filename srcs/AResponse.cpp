/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AResponse.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/03 13:52:46 by damachad          #+#    #+#             */
/*   Updated: 2024/09/04 18:05:40 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

AResponse::AResponse() {}

AResponse::~AResponse() {}

AResponse::AResponse(const ServerContext & server, const HTTP_Request & request)
	: _request(request), _server(server) {
	initializeStatusMessages();
}

AResponse::AResponse(const AResponse & src)
	: _request(src._request),
	 _server(src._server) {}

const AResponse & AResponse::operator=(const AResponse & src) {
	_request = src._request;
	_server = src._server;
}

// TODO: Review where to locate this info in the code
void AResponse::initializeStatusMessages(void) {
	_statusMessages[100] = "Continue";
	_statusMessages[101] = "Switching Protocols";
	
	_statusMessages[200] = "OK";
	_statusMessages[201] = "Created";
	_statusMessages[202] = "Accepted";
	_statusMessages[203] = "Non-Authoritative Information";
	_statusMessages[204] = "No Content";
	_statusMessages[205] = "Reset Content";
	_statusMessages[206] = "Partial Content";
	
	_statusMessages[300] = "Multiple Choices";
	_statusMessages[301] = "Moved Permanently";
	_statusMessages[302] = "Found";
	_statusMessages[303] = "See Other";
	_statusMessages[304] = "Not Modified";
	_statusMessages[305] = "Use Proxy";
	_statusMessages[307] = "Temporary Redirect";
	_statusMessages[308] = "Permanent Redirect";
	
	_statusMessages[400] = "Bad Request";
	_statusMessages[401] = "Unauthorized";
	_statusMessages[402] = "Payment Required";
	_statusMessages[403] = "Forbidden";
	_statusMessages[404] = "Not Found";
	_statusMessages[405] = "Method Not Allowed";
	_statusMessages[406] = "Not Acceptable";
	_statusMessages[407] = "Proxy Authentication Required";
	_statusMessages[408] = "Request Timeout";
	_statusMessages[409] = "Conflict";
	_statusMessages[410] = "Gone";
	_statusMessages[411] = "Length Required";
	_statusMessages[412] = "Precondition Failed";
	_statusMessages[413] = "Content Too Large";
	_statusMessages[414] = "URI Too Long";
	_statusMessages[415] = "Unsupported Media Type";
	_statusMessages[416] = "Range Not Satisfiable";
	_statusMessages[417] = "Expectation Failed";
	_statusMessages[421] = "Misdirected Request";
	_statusMessages[422] = "Unprocessable Content";
	_statusMessages[426] = "Upgrade Required";
	
	_statusMessages[500] = "Internal Server Error";
	_statusMessages[501] = "Not Implemented";
	_statusMessages[502] = "Bad Gateway";
	_statusMessages[503] = "Service Unavailable";
	_statusMessages[504] = "Gateway Timeout";
	_statusMessages[505] = "HTTP Version Not Supported";
}

std::string AResponse::getStatusMessage(short code) const {
	std::map<short, std::string>::const_iterator it = _statusMessages.find(code);
	if (it != _statusMessages.end()) {
		return it->second;
	} else
		return "Unknown Status";
}

short AResponse::isValidSize() const {
	// How to handle multiple Content-Length values ?
	if (_request.header_fields.count("Content-Length") > 1)
		return 400; // Bad Request
	std::multimap<std::string, std::string>::const_iterator it = _request.header_fields.find("Content-Length");
	long size = -1;
	if (it != _request.header_fields.end())
	{
		char *endPtr = NULL;
		size = std::strtol(it->second.c_str(), &endPtr, 10);
		if (*endPtr != '\0')
			return 400;
		if (size != _request.message_body.size())
			return 400;
	}
	if (size != _request.message_body.size())
			return 413; // Request Entity Too Large
	return 200; // mandatory Content-Length header ?
}

static int matchingChars(const std::string & url, const std::string & route) {
	size_t pos = url.find(route);
	if (pos == std::string::npos)
		return 0;
	return route.size();
}

LocationContext * AResponse::getMatchLocation(const std::string & url) {
	std::map<std::string, LocationContext> serverLocations = _server.getLocations();
	std::map<std::string, LocationContext>::iterator it;
	it = serverLocations.find(url);
	if (it != serverLocations.end())
		return &it->second;
	int biggestMatch = 0;
	LocationContext * closestLocation = NULL;
	for (it = serverLocations.begin(); it != serverLocations.end(); ++it) {
		int match = matchingChars(url, it->first);
		if (match > biggestMatch) {
			biggestMatch = match;
			closestLocation = &it->second;
		}
	}
	if (biggestMatch > 0)
		return closestLocation;
	return NULL;
}
