#pragma once
#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <map>
#include <signal.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "AResponse.hpp"
#include "HTTPRequestParser.hpp"

class CGI {
private:
  const HTTP_Request &_request;
  HTTP_Response &_response;
  const std::string &_path;

  // Checker
  bool isSingleValueHeader(std::string &key);

  // Env getters
  std::string getQueryFields();
  std::string getEnvVar(const char *key);
  std::string fetchCookies();

  // Env setter
  short setCGIEnv();

  // Parsers
  std::multimap<std::string, std::string>
  parseCGIHeaders(const std::string &headers);
  std::multimap<std::string, std::string> parseRequestHeaders();

public:
  CGI(HTTP_Request &httpRequest, HTTP_Response &httpResponse,
      const std::string &path);
  ~CGI();

  void handleCGIResponse();
  std::string executeCGI(const std::string &scriptPath);
  std::string getHeaderEnvValue(std::string key);
};
