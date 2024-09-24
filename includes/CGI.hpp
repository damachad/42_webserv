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

extern std::map<pid_t, time_t> pidStartTimeMap;

class CGI {
private:
  const HTTP_Request &_request;

  bool isSingleValueHeader(std::string &key);
  std::string getQueryFields();
  std::string getEnvVar(const char *key);
  std::string fetchCookies();
  void setCGIEnv();
  std::string getCGIScriptPath();
  std::multimap<std::string, std::string>
  parseCGIHeaders(const std::string &headers);
  std::multimap<std::string, std::string> parseRequestHeaders();

public:
  CGI(const HTTP_Request &httpRequest);
  ~CGI();

  std::string handleCGIResponse();
  std::string executeCGI(const std::string &scriptPath);
  std::string getHeaderEnvValue(std::string key);
};
