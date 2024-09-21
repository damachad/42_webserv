#pragma once
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <map>
#include <string>

#include "HTTPRequestParser.hpp"

class CGI {
   private:
	HTTP_Request &_request;

	bool isSingleValueHeader(std::string &key);
	std::string getQueryFields();
	std::string getHeaderEnvValue(std::string key);
	std::string getEnvVar(const char *key);
	std::string fetchCookies();
	std::string getCGIScriptPath();
	std::string executeCGI(const std::string &scriptPath);
	void setCGIEnv();
	std::multimap<std::string, std::string> parseCGIHeaders(
		const std::string &headers);

   public:
	CGI(HTTP_Request &httpRequest);
	~CGI();

	void handleCGIResponse();
};
