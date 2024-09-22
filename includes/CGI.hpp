#pragma once
#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "HTTPRequestParser.hpp"

class CGI {
   private:
	HTTP_Request &_request;

	bool isSingleValueHeader(std::string &key);
	std::string getQueryFields();
	std::string getHeaderEnvValue(std::string key);
	std::string getEnvVar(const char *key);
	std::string fetchCookies();
	void setCGIEnv();
	std::multimap<std::string, std::string> parseCGIHeaders(
		const std::string &headers);

   public:
	CGI(HTTP_Request &httpRequest);
	~CGI();

	std::string executeCGI(const std::string &scriptPath);
	std::string getCGIScriptPath();
	void handleCGIResponse();
	std::multimap<std::string, std::string> parseRequestHeaders();
};
