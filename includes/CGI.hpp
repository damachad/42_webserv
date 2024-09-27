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

#include "AResponse.hpp"
#include "HTTPRequestParser.hpp"

extern std::map<pid_t, time_t> pidStartTimeMap;

class CGI {
private:
  const HTTP_Request &_request;
  HTTP_Response &_response;
  const std::string &_path;

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
  CGI(const HTTP_Request &httpRequest, HTTP_Response &httpResponse,
      const std::string &path);
  ~CGI();

  void handleCGIResponse();
  std::string executeCGI(const std::string &scriptPath);
  std::string getHeaderEnvValue(std::string key);
};
