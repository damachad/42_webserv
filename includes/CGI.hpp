#pragma once
#include <cstdlib>
#include <map>
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
	void fetchCookies();
					  HTTP_Request &request);
};
