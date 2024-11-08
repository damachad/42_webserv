/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AResponse.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/03 13:52:46 by damachad          #+#    #+#             */
/*   Updated: 2024/11/02 11:45:29 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "AResponse.hpp"

// Initializes a map with status codes and returns it
static std::map<short, std::string> initStatusMessages() {
	std::map<short, std::string> m;
	m.insert(std::make_pair(100, "Continue"));
	m.insert(std::make_pair(101, "Switching Protocols"));

	m.insert(std::make_pair(200, "OK"));
	m.insert(std::make_pair(201, "Created"));
	m.insert(std::make_pair(202, "Accepted"));
	m.insert(std::make_pair(203, "Non-Authoritative Information"));
	m.insert(std::make_pair(204, "No Content"));
	m.insert(std::make_pair(205, "Reset Content"));
	m.insert(std::make_pair(206, "Partial Content"));

	m.insert(std::make_pair(300, "Multiple Choices"));
	m.insert(std::make_pair(301, "Moved Permanently"));
	m.insert(std::make_pair(302, "Found"));
	m.insert(std::make_pair(303, "See Other"));
	m.insert(std::make_pair(304, "Not Modified"));
	m.insert(std::make_pair(305, "Use Proxy"));
	m.insert(std::make_pair(307, "Temporary Redirect"));
	m.insert(std::make_pair(308, "Permanent Redirect"));

	m.insert(std::make_pair(400, "Bad Request"));
	m.insert(std::make_pair(401, "Unauthorized"));
	m.insert(std::make_pair(402, "Payment Required"));
	m.insert(std::make_pair(403, "Forbidden"));
	m.insert(std::make_pair(404, "Not Found"));
	m.insert(std::make_pair(405, "Method Not Allowed"));
	m.insert(std::make_pair(406, "Not Acceptable"));
	m.insert(std::make_pair(407, "Proxy Authentication Required"));
	m.insert(std::make_pair(408, "Request Timeout"));
	m.insert(std::make_pair(409, "Conflict"));
	m.insert(std::make_pair(410, "Gone"));
	m.insert(std::make_pair(411, "Length Required"));
	m.insert(std::make_pair(412, "Precondition Failed"));
	m.insert(std::make_pair(413, "Payload Too Large"));
	m.insert(std::make_pair(414, "URI Too Long"));
	m.insert(std::make_pair(415, "Unsupported Media Type"));
	m.insert(std::make_pair(416, "Range Not Satisfiable"));
	m.insert(std::make_pair(417, "Expectation Failed"));
	m.insert(std::make_pair(421, "Misdirected Request"));
	m.insert(std::make_pair(422, "Unprocessable Content"));
	m.insert(std::make_pair(426, "Upgrade Required"));

	m.insert(std::make_pair(500, "Internal Server Error"));
	m.insert(std::make_pair(501, "Not Implemented"));
	m.insert(std::make_pair(502, "Bad Gateway"));
	m.insert(std::make_pair(503, "Service Unavailable"));
	m.insert(std::make_pair(504, "Gateway Timeout"));
	m.insert(std::make_pair(505, "HTTP Version Not Supported"));
	return m;
}

// Global map of status codes and respective messages
const std::map<short, std::string> STATUS_MESSAGES = initStatusMessages();

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

AResponse::~AResponse() {}

AResponse::AResponse(const Server& server, const HTTP_Request& request)
	: _request(request), _server(server) {}

AResponse::AResponse(const AResponse& src)
	: _request(src._request),
	  _response(src._response),
	  _server(src._server),
	  _locationRoute(src._locationRoute) {}

// Checks if method is allowed in that location
short AResponse::checkMethod() const {
	const std::set<Method> allowedMethods =
		_server.getAllowedMethods(_locationRoute);
	std::set<Method>::const_iterator it = allowedMethods.find(_request.method);
	if (it == allowedMethods.end()) return METHOD_NOT_ALLOWED;
	return OK;
}

// Check if message body size is, at most, the maximum allowed body size
short AResponse::checkClientBodySize() const {
	size_t max_body_size = _server.getClientMaxBodySize();

	if (_request.message_body.size() > max_body_size) return 413;

	return 200;
}

// Sets locationRoute by matching uri to every location locationRoute is empty
// if there is no location match
//  NOTE: If REGEX is not considered, NGINX does prefix match for the location
// routes, which means route must match the start of the URI
void AResponse::setMatchLocationRoute() {
	std::map<std::string, Location> serverLocations = _server.getLocations();

	std::map<std::string, Location>::iterator it;
	it = serverLocations.find(_request.uri);
	if (it != serverLocations.end()) {
		_locationRoute = it->first;
		return;
	}

	size_t bestMatchLen = 0;
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

// Returns path to look for resource in location (root + uri)
const std::string AResponse::getPath() const {
	std::string root = _server.getRoot(_locationRoute);
	return (assemblePath(root, _request.uri));
}

// Checks if file is a regular file and there are no problems opening it
short AResponse::checkFile(const std::string& path) const {
	struct stat info;

	if (stat(path.c_str(), &info) != 0)	 // Error in getting file information
	{
		if (errno == ENOENT)
			return NOT_FOUND;
		else if (errno == EACCES)
			return FORBIDDEN;
	}
	bool expectDir = !path.empty() && (path.at(path.size() - 1) ==
									   '/');  // check if url ends with '/'
	if (expectDir && (info.st_mode & S_IFMT) != S_IFDIR) return NOT_FOUND;
	if ((info.st_mode & S_IFMT) != S_IFREG &&
		(info.st_mode & S_IFMT) != S_IFDIR)	 // Check if it is not regular file
											 // (not a link or device)
		return FORBIDDEN;
	return OK;
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

// Checks if the request is calling a CGI script
bool AResponse::isCGI() const {
	std::string cgiExt = _server.getCgiExt(_locationRoute);
	size_t dotPos = _request.uri.find_last_of(".");
	if (dotPos != std::string::npos && !cgiExt.empty() &&
		_request.uri.substr(dotPos) == cgiExt)
		return true;
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
		if (checkFile(filePath) == OK) return filePath;
	}
	return "";
}

// Joins both string and ensures there is a '/' in the middle
const std::string AResponse::assemblePath(const std::string& l,
										  const std::string& r) const {
	if (r.empty()) return l;
	if ((l.at(l.size() - 1) == '/' && r.at(0) != '/') ||
		(l.at(l.size() - 1) != '/' && r.at(0) == '/'))
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
			_response.headers.insert(
				std::make_pair(std::string("Content-Type"), it->second));
			return;
		}
	}
	_response.headers.insert(std::make_pair(
		std::string("Content-Type"),
		std::string("application/octet-stream")));	// default_type
}

// Adds Date, Server and Content-Length headers to _response
void AResponse::loadCommonHeaders() {
	_response.headers.insert(
		std::make_pair(std::string("Connection"), std::string("keep-alive")));
	if (_response.body.size()) {
		_response.headers.insert(std::make_pair(
			std::string("Content-Length"),
			numberToString<unsigned long>(_response.body.size())));
	}
	_response.headers.insert(
		std::make_pair(std::string("Date"), getHttpDate()));
	_response.headers.insert(
		std::make_pair(std::string("Server"), std::string(SERVER)));
	_response.headers.insert(
		std::make_pair(std::string("Cache-Control"), std::string("no-cache")));
}

// Loads reponse struct with values of return
void AResponse::loadReturn() {
	std::pair<short, std::string> redirect = _server.getReturn(_locationRoute);
	_response.body = redirect.second;
	_response.status = redirect.first;
	loadCommonHeaders();
	if (redirect.first == 301 || redirect.first == 302) {
		_response.headers.insert(
			std::make_pair(std::string("Location"), redirect.second));
	}
	_response.headers.insert(std::make_pair(
		std::string("Content-Type"), std::string("application/octet-stream")));
}

// Checks if there is a return directive
bool AResponse::hasReturn() const {
	std::pair<short, std::string> redirect = _server.getReturn(_locationRoute);
	if (redirect.first == -1 || redirect.second.empty()) return false;
	return true;
}

// Helper function to extract and format the directory name
static std::string getDirectoryName(const std::string& path) {
	std::string dirName;
	std::string::size_type endPos = path.find_last_not_of("/");
	if (endPos == std::string::npos) dirName = path;
	std::string::size_type pos = path.find_last_of("/", endPos);
	if (pos != std::string::npos) dirName = path.substr(pos, endPos - pos + 1);
	return dirName + "/";
}

// Returns last modified date of file
std::string AResponse::getLastModificationDate(const std::string& path) const {
	struct stat fileStat;
	if (stat(path.c_str(), &fileStat) != 0) return "";
	char dateBuffer[30];
	struct tm* timeinfo = gmtime(&fileStat.st_mtime);
	std::strftime(dateBuffer, sizeof(dateBuffer), "%d-%b-%Y %H:%M", timeinfo);
	return std::string(dateBuffer);
}

// Returns file's size or '-' if file is a directory
static std::string getFileSize(const std::string& path) {
	struct stat fileStat;
	if (stat(path.c_str(), &fileStat) != 0) return "";
	size_t size = fileStat.st_size;
	std::string sizeBuffer =
		(S_ISDIR(fileStat.st_mode) ? "-" : numberToString<size_t>(size));
	return sizeBuffer;
}

// Returns html line with file information to add to directory listing
std::string AResponse::addFileEntry(std::string& name,
									const std::string& path) {
	std::string fullPath = assemblePath(path, name);
	std::string date = getLastModificationDate(fullPath);
	std::string size = getFileSize(fullPath);
	std::string displayName;
	displayName = name;
	if (isDirectory(path)) displayName += '/';
	if (displayName.length() > 51)
		displayName = name.substr(0, 48) + "..>";  // Truncate and add ".."
	if (name != "./" && name != "../") name = assemblePath(_request.uri, name);
	std::string WP1;
	if (displayName.length() < 51)
		WP1 = std::string(51 - displayName.length(), ' ');	// Pad with spaces
	std::stringstream fileEntry;
	std::string WS2 = "                   ";
	if (displayName == "../")
		fileEntry << "<a href=\"" + name + "\">" + displayName + "</a> \n";
	else
		fileEntry << "<a href=\"" + name + "\">" + displayName + "</a> " + WP1 +
						 date + WS2 + size + "\n";
	return fileEntry.str();
}

// Loads response with a page containing directory listing for that location
short AResponse::loadDirectoryListing(const std::string& path) {
	DIR* dir = opendir(path.c_str());
	if (dir == NULL) return FORBIDDEN;
	std::string dirName = getDirectoryName(path);
	_response.body = "<!DOCTYPE html>\n<html>\n<head>\n<title>Index of " +
					 dirName + "</title>\n</head>\n<body>\n<h1>Index of " +
					 dirName + "</h1>\n<hr>\n<pre>";
	struct dirent* entry;
	std::vector<std::string> directories;
	std::vector<std::string> files;
	while ((entry = readdir(dir)) != NULL) {
		if (isDirectory(assemblePath(path, entry->d_name)))
			directories.push_back(std::string(entry->d_name));
		else
			files.push_back(std::string(entry->d_name));
	}
	// Sort the vectors alphabetically
	std::sort(directories.begin(), directories.end());
	std::sort(files.begin(), files.end());
	directories.erase(directories.begin());	 // Hide "."
	for (std::vector<std::string>::iterator it = directories.begin();
		 it != directories.end(); ++it) {
		std::string entryName = *it;
		_response.body += addFileEntry(entryName, path);
	}
	for (std::vector<std::string>::iterator it = files.begin();
		 it != files.end(); ++it) {
		std::string entryName = *it;
		_response.body += addFileEntry(entryName, path);
	}
	_response.body += "</pre>\n<hr>\n</body>\n</html>\n";
	closedir(dir);
	loadCommonHeaders();
	_response.headers.insert(
		std::make_pair(std::string("Content-Type"), std::string("text/html")));
	return OK;
}

// Converts the response struct into a string (loading the status message) and
// returns it
const std::string AResponse::getResponseStr() const {
	std::map<short, std::string>::const_iterator itStatus =
		STATUS_MESSAGES.find(_response.status);
	std::string message =
		(itStatus != STATUS_MESSAGES.end()) ? itStatus->second : "";
	std::string headersStr;
	std::multimap<std::string, std::string>::const_iterator itHead;
	for (itHead = _response.headers.begin(); itHead != _response.headers.end();
		 itHead++) {
		headersStr += itHead->first + ": " + itHead->second + "\r\n";
	}
	std::string response = "HTTP/1.1 " + numberToString<int>(_response.status) +
						   " " + message + "\r\n" + headersStr + "\r\n" +
						   _response.body;
	return response;
}

// Returns response as string with default error page body.
// Sets status and message
static std::string loadDefaultErrorPage(short status) {
	std::map<short, std::string>::const_iterator itStatus =
		STATUS_MESSAGES.find(status);
	std::string message =
		(itStatus != STATUS_MESSAGES.end()) ? itStatus->second : "";
	std::string response =
		"<!DOCTYPE html>\n"
		"<html lang=\"en\">\n"
		"<head>\n"
		"\t<meta charset=\"UTF-8\">\n"
		"\t<meta name=\"viewport\" content=\"width=device-width, "
		"initial-scale=1.0\">\n"
		"\t<title>" +
		message +
		"</title>\n"
		"\t<style>\n"
		"\t\th1, p {\n"
		"\t\t\ttext-align: center;\n"
		"\t\t}\n"
		"\t</style>\n"
		"</head>\n"
		"<body>\n"
		"\t<div>\n"
		"\t\t<h1>" +
		numberToString<short>(status) + " " + message +
		"</h1>\n"
		"<hr>\n"
		"\t\t<p>" +
		SERVER +
		"</p>\n"
		"\t</div>\n"
		"</body>\n"
		"</html>";
	return response;
}

// Loads response with respoective status code, gets personalized error page, if
// it exists and calls getResponseStr() to convert struct to string before
// returning it
const std::string AResponse::loadErrorPage(short status) {
	static std::map<short, std::string> error_pages =
		_server.getErrorPages(_locationRoute);
	_response.status = status;
	std::map<short, std::string>::const_iterator it = error_pages.find(status);
	if (it != error_pages.end()) {
		std::string path =
			assemblePath(_server.getRoot(_locationRoute), it->second);
		if (checkFile(path) == OK) {
			std::ifstream file(path.c_str());
			_response.body.assign((std::istreambuf_iterator<char>(file)),
								  (std::istreambuf_iterator<char>()));
		}
	}
	if (_response.body.empty()) _response.body = loadDefaultErrorPage(status);
	loadCommonHeaders();
	return getResponseStr();
}

// Returns "HTTP/1.1 100 Continue" string
const std::string AResponse::loadContinueMessage(void) {
	return "HTTP/1.1 100 Continue";
}

std::ostream& operator<<(std::ostream& outstream,
						 const HTTP_Response& response) {
	outstream << "HTTP Request: \n";

	outstream << "Status: " << response.status << " ";
	outstream << STATUS_MESSAGES.find(response.status)->second << "\n";
	outstream << "Header fields: \n";
	for (std::multimap<std::string, std::string>::const_iterator it =
			 response.headers.begin();
		 it != response.headers.end(); it++)
		outstream << "\t" << it->first << ": " << it->second << "\n";

	outstream << "Message body: " << response.body << std::endl;

	return outstream;
}
