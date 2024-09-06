/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AResponse.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/03 13:52:46 by damachad          #+#    #+#             */
/*   Updated: 2024/09/06 16:38:26 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

const std::map<short, std::string> STATUS_MESSAGES = {
    {100, "Continue"},
    {101, "Switching Protocols"},
    {200, "OK"},
    {201, "Created"},
    {202, "Accepted"},
    {203, "Non-Authoritative Information"},
    {204, "No Content"},
    {205, "Reset Content"},
    {206, "Partial Content"},
    {300, "Multiple Choices"},
    {301, "Moved Permanently"},
    {302, "Found"},
    {303, "See Other"},
    {304, "Not Modified"},
    {305, "Use Proxy"},
    {307, "Temporary Redirect"},
    {308, "Permanent Redirect"},
    {400, "Bad Request"},
    {401, "Unauthorized"},
    {402, "Payment Required"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {405, "Method Not Allowed"},
    {406, "Not Acceptable"},
    {407, "Proxy Authentication Required"},
    {408, "Request Timeout"},
    {409, "Conflict"},
    {410, "Gone"},
    {411, "Length Required"},
    {412, "Precondition Failed"},
    {413, "Content Too Large"},
    {414, "URI Too Long"},
    {415, "Unsupported Media Type"},
    {416, "Range Not Satisfiable"},
    {417, "Expectation Failed"},
    {421, "Misdirected Request"},
    {422, "Unprocessable Content"},
    {426, "Upgrade Required"},
    {500, "Internal Server Error"},
    {501, "Not Implemented"},
    {502, "Bad Gateway"},
    {503, "Service Unavailable"},
    {504, "Gateway Timeout"},
    {505, "HTTP Version Not Supported"}
};

AResponse::AResponse() {}

AResponse::~AResponse() {}

AResponse::AResponse(const ServerContext & server, const HTTP_Request & request)
	: _request(request), _server(server) {
}

AResponse::AResponse(const AResponse & src)
	: _request(src._request),
	 _response(src._response),
	 _server(src._server),
	 _locationRoute(src._locationRoute) {}

const AResponse & AResponse::operator=(const AResponse & src) {
	_request = src._request;
	_response = src._response;
	_server = src._server;
	_locationRoute = src._locationRoute;
}

void AResponse::checkSize() const {
	if (_request.header_fields.count("Content-Length") == 0) // mandatory Content-Length header ?
		throw HTTPResponseError(411); // Length Required
	if (_request.message_body.size() > _server.getClientMaxBodySize(_locationRoute))
		throw HTTPResponseError(413); // Request Entity Too Large
	// How to handle multiple Content-Length values ?
	if (_request.header_fields.count("Content-Length") > 1)
		throw HTTPResponseError(400); // Bad Request
	std::multimap<std::string, std::string>::const_iterator it = _request.header_fields.find("Content-Length");
	long size = -1;
	if (it != _request.header_fields.end())
	{
		char *endPtr = NULL;
		size = std::strtol(it->second.c_str(), &endPtr, 10);
		if (*endPtr != '\0')
			throw HTTPResponseError(400);
		if (size != _request.message_body.size())
			throw HTTPResponseError(400);
	}
}

// TODO: Change method in HTTP_Request to Method
void AResponse::checkMethod() const {
	std::set<Method>::const_iterator it = _server.getAllowedMethods(_locationRoute).find(_request.method);
	if (it == _server.getAllowedMethods(_locationRoute).end())
		throw HTTPResponseError(405); // Method Not Allowed
}

// If REGEX is not considered, NGINX does prefix match for the location routes,
// which means route must match the start of the URI
void AResponse::getMatchLocationRoute() {
	std::map<std::string, LocationContext> serverLocations = _server.getLocations();
	
	std::map<std::string, LocationContext>::iterator it;
	it = serverLocations.find(_request.uri);
	if (it != serverLocations.end()) {
		_locationRoute = it->first;
		return ;
	}

	int bestMatchLen = 0;
	std::string bestMatchRoute;
	for (it = serverLocations.begin(); it != serverLocations.end(); ++it) {
		if (_request.uri.compare(0, it->first.size(), it->first) == 0) {
			size_t match = it->first.size();
			if (match > bestMatchLen) {
				bestMatchLen = match;
				bestMatchRoute = it->first;
			}
		}
	}
	_locationRoute = bestMatchRoute;
}

const std::string & AResponse::getPath() {
	std::string root = _server.getRoot(_locationRoute);
	return (root + _request.uri.substr(_locationRoute.size()));
}

// TODO: Review check empty logic
void AResponse::checkReturn() const {
	std::pair<short, std::string> redirect = _server.getReturn(_locationRoute);
	if (redirect.second.empty())
		return ;
	if (redirect.first == 302) {
		
	}
		
}

bool isDirectory(const char* path) {
	struct stat info;

	if (stat(path, &info) != 0) {
		// Error in getting file information (file does not exist, permission denied, etc.)
		return false;
	}
	return (info.st_mode & S_IFDIR) != 0;  // Check if it's a directory
}

// TODO: finish implementing (get contents from error page or default if none)
std::string & AResponse::getErrorPage(short status) {
	std::map<short, std::string>::const_iterator it;
	it = _server.getErrorPages(_locationRoute).find(status);
	if (it !=  _server.getErrorPages(_locationRoute).end()) {
		// Get content from specified file (check file first)
		return ;
	}
}

std::string & AResponse::getResponseStr() const {
	std::map<short, std::string>::const_iterator itStatus = STATUS_MESSAGES.find(_response.status);
	std::string message = (itStatus != STATUS_MESSAGES.end()) ? itStatus->second : "Unknown status code";
	std::string headersStr;
	std::multimap<std::string, std::string>::const_iterator itHead;
	for (itHead = _response.headers.begin(); itHead != _response.headers.end(); itHead++) {
		headersStr += itHead->first + ": " + itHead->second + "\r\n";
	}
	std::string response = "HTTP/1.1 " + int_to_string(_response.status)
	 + message + "\r\n" + headersStr + "\r\n" + _response.body;
	return response;
}

// Example implementation
// std::string generateResponse() {
// 	setMatchLocationRoute();
// 	checkSize();
// 	checkMethod();
// 	checkReturn();
// 	std::string path = getPath();
// 
//	Check if file exists and is not a dir
// 	
// 	return getResponseStr();
// }
