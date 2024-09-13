#pragma once
#include <cstdlib>
#include <map>
#include <string>

class CGI {
   private:
	std::map<const std::string, std::string> _CGIEnv;

   public:
	CGI();
	~CGI();

	std::string getCGIEnv(std::string key);

	std::string getEnvVar(const char *key);
};
