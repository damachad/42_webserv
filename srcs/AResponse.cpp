/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AResponse.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/03 13:52:46 by damachad          #+#    #+#             */
/*   Updated: 2024/09/10 11:50:38 by damachad         ###   ########.fr       */
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
	{505, "HTTP Version Not Supported"}};

AResponse::AResponse() {}

AResponse::~AResponse() {}

AResponse::AResponse(const ServerContext& server, const HTTP_Request& request)
	: _request(request), _server(server) {}

AResponse::AResponse(const AResponse& src)
	: _request(src._request),
	  _response(src._response),
	  _server(src._server),
	  _locationRoute(src._locationRoute) {}

const AResponse& AResponse::operator=(const AResponse& src) {
	_request = src._request;
	_response = src._response;
	_server = src._server;
	_locationRoute = src._locationRoute;
}

void AResponse::checkSize() const {
	if (_request.header_fields.count("Content-Length") ==
		0)							   // mandatory Content-Length header ?
		throw HTTPResponseError(411);  // Length Required
	if (_request.message_body.size() >
		_server.getClientMaxBodySize(_locationRoute))
		throw HTTPResponseError(413);  // Request Entity Too Large
	// How to handle multiple Content-Length values ?
	if (_request.header_fields.count("Content-Length") > 1)
		throw HTTPResponseError(400);  // Bad Request
	std::multimap<std::string, std::string>::const_iterator it =
		_request.header_fields.find("Content-Length");
	long size = -1;
	if (it != _request.header_fields.end()) {
		char* endPtr = NULL;
		size = std::strtol(it->second.c_str(), &endPtr, 10);
		if (*endPtr != '\0') throw HTTPResponseError(400);
		if (size != _request.message_body.size()) throw HTTPResponseError(400);
	}
}

void AResponse::checkMethod() const {
	std::set<Method>::const_iterator it =
		_server.getAllowedMethods(_locationRoute).find(_request.method);
	if (it == _server.getAllowedMethods(_locationRoute).end())
		throw HTTPResponseError(405);  // Method Not Allowed
}

// If REGEX is not considered, NGINX does prefix match for the location routes,
// which means route must match the start of the URI
void AResponse::setMatchLocationRoute() {
	std::map<std::string, LocationContext> serverLocations =
		_server.getLocations();

	std::map<std::string, LocationContext>::iterator it;
	it = serverLocations.find(_request.uri);
	if (it != serverLocations.end()) {
		_locationRoute = it->first;
		return;
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

const std::string& AResponse::getPath() const {
	std::string root = _server.getRoot(_locationRoute);
	// TODO: ensure there is one '/' present in between root and URI
	return (assemblePath(root, _request.uri.substr(_locationRoute.size())));
}

// TODO: Review check empty logic
void AResponse::checkReturn() const {
	std::pair<short, std::string> redirect = _server.getReturn(_locationRoute);
	if (redirect.second.empty()) return;
	if (redirect.first == 302) {
		// TODO: Implement redirect logic
	}
}

short AResponse::checkFile(const std::string& path) const {
	struct stat info;

	if (stat(path.c_str(), &info) != 0)	 // Error in getting file information
	{
		if (errno == ENOENT)
			throw HTTPResponseError(404);  // file does not exist
		else if (errno == EACCES)
			throw HTTPResponseError(403);  // permission denied
		else
			throw HTTPResponseError(500);  // TODO: Check if necessary later
		return -1;
	} else if ((info.st_mode & S_IFMT) ==
			   S_IFDIR)	 // Check if it is a directory
		return 1;
	else if ((info.st_mode & S_IFMT) ==
			 S_IFREG)  // Check if it is a regular file (link, device, etc.)
		return 0;
	else {
		throw HTTPResponseError(403);  // permission denied
		return -2;
	}
}

bool AResponse::hasAutoindex() const {
	if (_server.getAutoIndex(_locationRoute) == TRUE)
		return true;
	else
		return false;
}

std::string AResponse::getIndexFile(const std::string& path) const {
	std::vector<std::string> indexFiles = _server.getIndex(_locationRoute);
	std::vector<std::string>::const_iterator it;
	for (it = indexFiles.begin(); it != indexFiles.end(); it++) {
		std::string filePath = assemblePath(path, *it);
		if (checkFile(filePath) == REG_FILE)
			return filePath;
		else if (checkFile(filePath) == DIRECTORY)
			throw HTTPResponseError(403);  // TODO: NGINX deals with index file
										   // being a dir by doing a redirection
	}
	return NULL;
}

// TODO: finish implementing (get contents from error page or default if none)
std::string& AResponse::getErrorPage(short status) const {
	std::map<short, std::string>::const_iterator it;
	it = _server.getErrorPages(_locationRoute).find(status);
	if (it != _server.getErrorPages(_locationRoute).end()) {
		// Get content from specified file (check file first)
		return;
	}
}

// TODO: review edge cases (double '/')
std::string AResponse::assemblePath(const std::string& l,
									const std::string& r) const {
	if ((l.back() == '/' && r.at(0) != '/') ||
		(l.back() != '/' && r.at(0) == '/'))
		return l + r;
	else
		return l + '/' + r;
}

// Initialize a map with common MIME types and returns it
static std::map<std::string, std::string> initMimeTypes() {
	std::map<std::string, std::string> mimeTypes;

	// Text formats
	mimeTypes["html"] = "text/html";
	mimeTypes["htm"] = "text/html";
	mimeTypes["css"] = "text/css";
	mimeTypes["csv"] = "text/csv";
	mimeTypes["txt"] = "text/plain";
	mimeTypes["xml"] = "application/xml";

	// Image formats
	mimeTypes["png"] = "image/png";
	mimeTypes["jpg"] = "image/jpeg";
	mimeTypes["jpeg"] = "image/jpeg";
	mimeTypes["gif"] = "image/gif";
	mimeTypes["bmp"] = "image/bmp";
	mimeTypes["ico"] = "image/x-icon";

	// Audio/Video formats
	mimeTypes["mp3"] = "audio/mpeg";
	mimeTypes["wav"] = "audio/wav";
	mimeTypes["mp4"] = "video/mp4";
	mimeTypes["avi"] = "video/x-msvideo";
	mimeTypes["mov"] = "video/quicktime";

	// Application formats
	mimeTypes["json"] = "application/json";
	mimeTypes["js"] = "application/javascript";
	mimeTypes["pdf"] = "application/pdf";
	mimeTypes["zip"] = "application/zip";
	mimeTypes["tar"] = "application/x-tar";
	mimeTypes["gz"] = "application/gzip";
	mimeTypes["exe"] = "application/octet-stream";
	mimeTypes["bin"] = "application/octet-stream";

	// Microsoft formats
	mimeTypes["doc"] = "application/msword";
	mimeTypes["docx"] =
		"application/"
		"vnd.openxmlformats-officedocument.wordprocessingml.document";
	mimeTypes["xls"] = "application/vnd.ms-excel";
	mimeTypes["xlsx"] =
		"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
	mimeTypes["ppt"] = "application/vnd.ms-powerpoint";
	mimeTypes["pptx"] =
		"application/"
		"vnd.openxmlformats-officedocument.presentationml.presentation";

	// Other formats
	mimeTypes["svg"] = "image/svg+xml";
	mimeTypes["woff"] = "font/woff";
	mimeTypes["woff2"] = "font/woff2";

	return mimeTypes;
}

// Function to get MIME type based on file extension
void AResponse::setMimeType(const std::string& path) {
	static std::map<std::string, std::string> mimeTypes = initMimeTypes();

	std::size_t dotPos = path.find_last_of('.');
	if (dotPos != std::string::npos) {
		std::string extension = path.substr(dotPos + 1);
		std::map<std::string, std::string>::const_iterator it =
			mimeTypes.find(extension);
		if (it != mimeTypes.end()) {
			_response.headers.insert(std::string("Content-Type"), it->second);
			return;
		}
	}
	_response.headers.insert(
		std::string("Content-Type"),
		std::string("application/octet-stream"));  // default_type
}

void AResponse::loadCommonHeaders() {
	_response.headers.insert(std::string("Date"), getHttpDate());
	_response.headers.insert(std::string("Server"), std::string(SERVER));
}

void AResponse::loadDirectoryListing(const std::string& path) {
	DIR* dir = opendir(path.c_str());
	if (dir == NULL) throw HTTPResponseError(403);
	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		_response.body +=
			entry->d_name + '\n';  // Print the name of each file/directory
	}
	closedir(dir);
	loadCommonHeaders();
	_response.headers.insert(std::string("Content-Length"),
							 int_to_string(_response.body.size()));
	_response.headers.insert(std::string("Content-Type"),
							 std::string("text/html"));
	_response.status = 200;
}

// for GET
void AResponse::loadFile(const std::string& path) {
	std::ifstream file(path.c_str());
	if (!file.is_open()) throw HTTPResponseError(500);
	_response.body.assign((std::istreambuf_iterator<char>(file)),
						  (std::istreambuf_iterator<char>()));
	file.close();
	loadCommonHeaders();
	_response.headers.insert(std::string("Content-Length"),
							 int_to_string(_response.body.size()));
	setMimeType(path);
	_response.status = 200;
}

std::string& AResponse::getResponseStr() const {
	std::map<short, std::string>::const_iterator itStatus =
		STATUS_MESSAGES.find(_response.status);
	std::string message = (itStatus != STATUS_MESSAGES.end())
							  ? itStatus->second
							  : "Unknown status code";
	std::string headersStr;
	std::multimap<std::string, std::string>::const_iterator itHead;
	for (itHead = _response.headers.begin(); itHead != _response.headers.end();
		 itHead++) {
		headersStr += itHead->first + ": " + itHead->second + "\r\n";
	}
	std::string response = "HTTP/1.1 " + int_to_string(_response.status) +
						   message + "\r\n" + headersStr + "\r\n" +
						   _response.body;
	return response;
}

// Example implementation
// std::string generateResponse() {
// 	setMatchLocationRoute();
// 	checkSize();
// 	checkMethod();
// 	checkReturn();
// 	std::string path = getPath();

// 	if (checkFile(path) == REG_FILE)  // file exists and is not a dir
// 		loadFile(path); // if GET
// 	else if (checkFile(path) == DIRECTORY) {
// 		std::string indexFile = checkIndex(path);
// 		if (!indexFile.empty())
// 			loadFile(indexFile); // if GET
// 		else if (hasAutoindex())
// 			loadDirectoryListing();
// 		else
// 			throw HTTPResponseError(404);
// 	}
//  // Sub classes should change the status code accordingly, etc.

// 	return getResponseStr();
// }
