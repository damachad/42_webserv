#pragma once
#include <cstdlib>
#include <map>
#include <string>

#include "HTTPRequestParser.hpp"

class CGI {
   private:
	std::map<const std::string, std::string> _CGIEnv;
	HTTP_Request_Parser *request;

   public:
	CGI();
	~CGI();

	std::string getCGIEnv(std::string key);

	std::string getEnvVar(const char *key);
};
