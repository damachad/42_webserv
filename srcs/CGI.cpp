#include "CGI.hpp"

CGI::CGI(HTTP_Request &httpRequest) : _request(httpRequest) {}

CGI::~CGI() {}

bool CGI::isSingleValueHeader(std::string &key) {
	if (key == "Accept" || key == "Accept-Encoding" || key == "Cache-Control" ||
		key == "Set-Cookie" || key == "Via" || key == "Forewarded")
		return false;
	return true;
}

std::string CGI::getQueryFields() {
	std::string queryString;
	for (std::multimap<std::string, std::string>::const_iterator it =
			 _request.query_fields.begin();
		 it != _request.query_fields.end(); ++it) {
		if (!queryString.empty()) queryString += "&";
		queryString += it->first + "=" + it->second;
	}
	return queryString;
}

std::string CGI::getHeaderEnvValue(std::string key) {
	// Find the range of values associated with the key
	std::pair<std::multimap<std::string, std::string>::iterator,
			  std::multimap<std::string, std::string>::iterator>
		range;
	range = _request.header_fields.equal_range(key);

	if (isSingleValueHeader(key)) {
		// If there are values associated with the key, return the first one
		if (range.first != range.second) {
			return range.first->second;	 // Return the first matching value
		}
	} else {
		std::string result;
		for (std::multimap<std::string, std::string>::iterator it = range.first;
			 it != range.second; ++it) {
			if (!result.empty()) {
				result += ", ";
			}
			result += it->second;
		}

		return result;
	}
	return "";	// Return an empty string if the key is not found
}

std::string CGI::getEnvVar(const char *key) {
	const char *pair = getenv(key);
	return (pair != NULL) ? std::string(pair) : "";
}

std::string CGI::fetchCookies() {
	std::string result;

	for (std::multimap<std::string, std::string>::const_iterator it =
			 _request.header_fields.begin();
		 it != _request.header_fields.end(); ++it) {
		if (it->first == "Cookie") {
			if (!result.empty()) {
				result += "; ";
			}
			result += it->second;
		}
	}

	return result;
}

void CGI::setCGIEnv() {
	setenv("REQUEST_METHOD", std::to_string(_request.method).c_str(), 1);
	setenv("CONTENT_TYPE", getHeaderEnv("Content-Type").c_str(), 1);
	setenv("CONTENT_LENGTH", getHeaderEnvValue("Content-Length").c_str(), 1);
	setenv("QUERY_STRING", getQueryFields().c_str(), 1);
	setenv("SCRIPT_NAME", _request.uri.c_str(), 1);
	setenv("SERVER_PROTOCOL", _request.protocol_version.c_str(), 1);
	setenv("HTTP_COOKIE", )
}
