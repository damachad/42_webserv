#include "CGI.hpp"

CGI::CGI() {
  _CGIEnv["CONTENT_TYPE"] = getEnvVar("CONTENT_TYPE");
  _CGIEnv["CONTENT_LENGTH"] = getEnvVar("CONTENT_TYPE");
  _CGIEnv["HTTP_COOKIE"] = getEnvVar("CONTENT_TYPE");
  _CGIEnv["HTTP_USER_AGENT"] = getEnvVar("CONTENT_TYPE");
  _CGIEnv["PATH_INFO"] = getEnvVar("CONTENT_TYPE");
  _CGIEnv["QUERY_STRING"] = getEnvVar("CONTENT_TYPE");
  _CGIEnv["REMOTE_ADDR"] = getEnvVar("CONTENT_TYPE");
  _CGIEnv["REMOTE_HOST"] = getEnvVar("CONTENT_TYPE");
  _CGIEnv["REQUEST_METHOD"] = getEnvVar("CONTENT_TYPE");
  _CGIEnv["SCRIPT_FILENAME"] = getEnvVar("CONTENT_TYPE");
  _CGIEnv["SCRIPT_NAME"] = getEnvVar("CONTENT_TYPE");
  _CGIEnv["SERVER_NAME"] = getEnvVar("CONTENT_TYPE");
  _CGIEnv["SERVER_SOFTWARE"] = getEnvVar("CONTENT_TYPE");
}

CGI::~CGI() {}

std::string CGI::getCGIEnv(std::string key) { return _CGIEnv[key]; }

void CGI::setCGIEnv(std::string key, std::string pair) { _CGIEnv[key] = pair; }

std::string CGI::getEnvVar(const char *key) {
  const char *pair = getenv(key);
  return (pair != NULL) ? std::string(pair) : "";
}
