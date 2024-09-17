#include "CGI.hpp"

CGI::CGI(HTTP_Request &httpRequest) : _request(httpRequest) {}

CGI::~CGI() {}

bool isSingleValueHeader(std::string &key) {
	if (key == "Accept" || key == "Accept-Encoding" || key == "Cache-Control" ||
		key == "Set-Cookie" || key == "Via" || key == "Forewarded")
		return false;
	return true;
}

void CGI::fetchCookies(std::multimap<std::string, std::string> &headerEnv,
					   HTTP_Request &request) {
	for (std::multimap<std::string, std::string>::const_iterator it =
			 request.header_fields.begin();
		 it != request.header_fields.end(); ++it) {
		if (it->first == "Set-Cookie") {
			std::string cookieHeader = it->second;

			size_t pos = cookieHeader.find('=');
			if (pos != std::string::npos) {
				std::string cookieName = cookieHeader.substr(0, pos);
				std::string cookieValue = cookieHeader.substr(pos + 1);

				headerEnv.insert(std::make_pair(cookieName, cookieValue));
			}
		}
	}
}

std::string CGI::getHeaderEnv(std::string key) {
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
