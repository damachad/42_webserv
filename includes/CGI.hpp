#pragma once
#include <unistd.h>

#include <cstdlib>
#include <map>
#include <stdexcept>
#include <string>

#include "HTTPRequestParser.hpp"

class CGI {
   private:
	std::multimap<const std::string, std::string> _headerEnv;
	HTTP_Request &_request;

	bool isSingleValueHeader(std::string &key);
	std::string getQueryFields();

   public:
	CGI(HTTP_Request &httpRequest);
	~CGI();

	std::string getHeaderEnvValue(std::string key);
	std::string getEnvVar(const char *key);
	std::string fetchCookies();
	void setCGIEnv();
	std::string executeCGI(const std::string &scriptPath);
};
