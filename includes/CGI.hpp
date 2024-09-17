#pragma once
#include <cstdlib>
#include <map>
#include <string>

#include "HTTPRequestParser.hpp"

class CGI {
   private:
	std::multimap<const std::string, std::string> _headerEnv;
	HTTP_Request &_request;

   public:
	CGI(HTTP_Request &httpRequest);
	~CGI();

	std::string getHeaderEnv(std::string key);

	std::string getEnvVar(const char *key);

	void fetchCookies(std::multimap<std::string, std::string> &headerEnv,
					  HTTP_Request &request);
};
