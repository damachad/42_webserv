/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AResponse.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/03 13:52:46 by damachad          #+#    #+#             */
/*   Updated: 2024/09/11 12:15:23 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

// Global map of status codes and respective messages
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

// Initializes a map with common MIME types and returns it
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

// Checks if Content-Lenght is present once and if request body size matches
// this value and value of client_max_body_size
short AResponse::checkSize() const {
	if (_request.header_fields.count("Content-Length") ==
		0)			 // mandatory Content-Length header ?
		return 411;	 // Length Required
	if (_request.message_body.size() >
		_server.getClientMaxBodySize(_locationRoute))
		return 413;	 // Request Entity Too Large
	// How to handle multiple Content-Length values ?
	if (_request.header_fields.count("Content-Length") > 1)
		return 400;	 // Bad Request
	std::multimap<std::string, std::string>::const_iterator it =
		_request.header_fields.find("Content-Length");
	long size = -1;
	if (it != _request.header_fields.end()) {
		char* endPtr = NULL;
		size = std::strtol(it->second.c_str(), &endPtr, 10);
		if (*endPtr != '\0') return 400;
		if (size != _request.message_body.size()) return 400;
	}
	return 200;
}

// Checks if method is allowed in that location
short AResponse::checkMethod() const {
	std::set<Method>::const_iterator it =
		_server.getAllowedMethods(_locationRoute).find(_request.method);
	if (it == _server.getAllowedMethods(_locationRoute).end())
		return 405;	 // Method Not Allowed
	return 200;
}

// Sets locationRoute by matching uri to every location locationRoute is empty
// if there is no location match
//  NOTE: If REGEX is not considered, NGINX does prefix match for the location
// routes, which means route must match the start of the URI
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

// Returns path to look for resource in location, root + (uri - locationRoute)
const std::string& AResponse::getPath() const {
	std::string root = _server.getRoot(_locationRoute);
	return (assemblePath(root, _request.uri.substr(_locationRoute.size())));
}

// Checks if file is a regular file and there are no problems opening it
short AResponse::checkFile(const std::string& path) const {
	struct stat info;

	if (stat(path.c_str(), &info) != 0)	 // Error in getting file information
	{
		if (errno == ENOENT)
			return 404;	 // file does not exist
		else if (errno == EACCES)
			return 403;	 // permission denied
		else
			return 500;	 // TODO: Check if necessary later
	} else if ((info.st_mode & S_IFMT) != S_IFDIR &&
			   (info.st_mode & S_IFMT) !=
				   S_IFREG)	 // Check if it is a directory or a regular file
							 // (not a link or device)
		return 403;			 // permission denied
	return 200;
}

// Checks if file (path) is a directory
bool AResponse::isDirectory(const std::string& path) const {
	struct stat info;

	stat(path.c_str(), &info);
	if ((info.st_mode & S_IFMT) == S_IFDIR)	 // Check if it is a directory
		return true;
	else
		return false;
}

// Checks if autoindex is on
bool AResponse::hasAutoindex() const {
	if (_server.getAutoIndex(_locationRoute) == TRUE)
		return true;
	else
		return false;
}

// Returns an index file if it exists or NULL (empty string)
const std::string AResponse::getIndexFile(const std::string& path) const {
	std::vector<std::string> indexFiles = _server.getIndex(_locationRoute);
	std::vector<std::string>::const_iterator it;
	for (it = indexFiles.begin(); it != indexFiles.end(); it++) {
		std::string filePath = assemblePath(path, *it);
		if (checkFile(filePath) == 200) return filePath;
	}
	return NULL;
}

// Joins both string and ensures there is a '/' in the middle
//  TODO: review edge cases (double '/')
const std::string AResponse::assemblePath(const std::string& l,
										  const std::string& r) const {
	if ((l.back() == '/' && r.at(0) != '/') ||
		(l.back() != '/' && r.at(0) == '/'))
		return l + r;
	else
		return l + '/' + r;
}

// Sets MIME type in Content-Type header based on file extension
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

// Adds Date, Server and Content-Length headers to _response
void AResponse::loadCommonHeaders() {
	_response.headers.insert(std::string("Date"), getHttpDate());
	_response.headers.insert(std::string("Server"), std::string(SERVER));
	_response.headers.insert(std::string("Content-Length"),
							 int_to_string(_response.body.size()));
}

// Loads reponse struct with values of return
void AResponse::loadReturn() {
	std::pair<short, std::string> redirect = _server.getReturn(_locationRoute);
	_response.body = redirect.second;
	_response.status = redirect.first;
	loadCommonHeaders();
	if (redirect.first == 301 || redirect.first == 302) {
		_response.headers.insert(std::string("Location"), redirect.second);
	}
}

// Checks if there is a return directive
//  TODO: Review check empty logic
bool AResponse::hasReturn() const {
	std::pair<short, std::string> redirect = _server.getReturn(_locationRoute);
	if (redirect.second.empty()) return false;
	return true;
}

// Loads response with a page containing directory listing for that location
short AResponse::loadDirectoryListing(const std::string& path) {
	DIR* dir = opendir(path.c_str());
	if (dir == NULL) return 403;
	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		_response.body +=
			entry->d_name + '\n';  // Print the name of each file/directory
	}
	closedir(dir);
	loadCommonHeaders();
	_response.headers.insert(std::string("Content-Type"),
							 std::string("text/html"));
	_response.status = 200;
	return 200;
}

// Loads response with contents of file and sets MIME type
//  NOTE: Used for GET
// short AResponse::loadFile(const std::string& path) {
// 	std::ifstream file(path.c_str());
// 	if (!file.is_open()) return 500;
// 	_response.body.assign((std::istreambuf_iterator<char>(file)),
// 						  (std::istreambuf_iterator<char>()));
// 	file.close();
// 	loadCommonHeaders();
// 	setMimeType(path);
// 	_response.status = 200;
// 	return 200;
// }

// Converts the response struct into a string (loading the status message) and
// returns it
const std::string& AResponse::getResponseStr() const {
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

// Loads response with respoective status code, gets personalized error page, if
// it exists and calls getResponseStr() to convert struct to string before
// returning it
const std::string& AResponse::loadErrorPage(short status) {
	static std::map<short, std::string> error_pages =
		_server.getErrorPages(_locationRoute);
	_response.status = status;
	std::map<short, std::string>::const_iterator it = error_pages.find(status);
	if (it != error_pages.end()) {
		std::string path =
			assemblePath(_server.getRoot(_locationRoute), it->second);
		if (checkFile(path) == 200) {
			std::ifstream file(path.c_str());
			_response.body.assign((std::istreambuf_iterator<char>(file)),
								  (std::istreambuf_iterator<char>()));
		}
	}
	loadCommonHeaders();
	return getResponseStr();
}

// Example implementation (case GET)
// Order of functions is important
std::string AResponse::generateResponse() {
	setMatchLocationRoute();
	short status = checkSize();
	if (status != 200) return loadErrorPage(status);
	status = checkMethod();
	if (status != 200) return loadErrorPage(status);
	if (hasReturn()) {
		loadReturn();
		return getResponseStr();
	}
	std::string path = getPath();

	status = checkFile(path);
	if (status != 200) return loadErrorPage(status);
	if (!isDirectory(path)) {
		// status = loadFile(path);  // if GET
		if (status != 200) return loadErrorPage(status);
	} else {  // is a directory
		std::string indexFile = getIndexFile(path);
		if (!indexFile.empty() &&
			!isDirectory(indexFile)) {	// TODO: deal with directory in index?
			// status = loadFile(indexFile);  // if GET
			if (status != 200) return loadErrorPage(status);
		} else if (hasAutoindex()) {
			status = loadDirectoryListing(path);
			if (status != 200) return loadErrorPage(status);
		} else
			loadErrorPage(404);
	}

	return getResponseStr();
}
