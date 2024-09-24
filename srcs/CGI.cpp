#include "CGI.hpp"
#include <sys/wait.h>

std::map<pid_t, time_t> pidStartTimeMap;

CGI::CGI(const HTTP_Request &httpRequest) : _request(httpRequest) {}

CGI::~CGI() {}

bool CGI::isSingleValueHeader(std::string &key) {
  if (key == "Accept" || key == "Accept-Encoding" || key == "Cache-Control" ||
      key == "Set-Cookie" || key == "Via" || key == "Forewarded")
    return false;
  return true;
}

std::string CGI::getQueryFields() {
  std::string queryString;
  for (std::multimap<std::string, std::string>::const_iterator it =
           _request.query_fields.begin();
       it != _request.query_fields.end(); ++it) {
    if (!queryString.empty())
      queryString += "&";
    queryString += it->first + "=" + it->second;
  }
  return queryString;
}

std::string CGI::getHeaderEnvValue(std::string key) {
  std::pair<std::multimap<std::string, std::string>::const_iterator,
            std::multimap<std::string, std::string>::const_iterator>
      range = _request.header_fields.equal_range(key);

  if (isSingleValueHeader(key)) {
    if (range.first != range.second) {
      return range.first->second;
    }
  } else {
    std::string result;
    for (std::multimap<std::string, std::string>::const_iterator it = range.first;
         it != range.second; ++it) {
      if (!result.empty()) {
        result += ", ";
      }
      result += it->second;
    }
    return result;
  }
  return "";
}

std::string CGI::getEnvVar(const char *key) {
  const char *pair = getenv(key);
  return (pair != NULL) ? std::string(pair) : "";
}

std::string CGI::fetchCookies() {
  std::string result;
  for (std::multimap<std::string, std::string>::const_iterator it =
           _request.header_fields.begin();
       it != _request.header_fields.end(); ++it) {
    if (it->first == "Cookie") {
      if (!result.empty()) {
        result += "; ";
      }
      result += it->second;
    }
  }
  return result;
}

std::string intToString(int value) {
  std::stringstream ss;
  ss << value;
  return ss.str();
}

void CGI::setCGIEnv() {
  try {
    if (setenv("REQUEST_METHOD", intToString(_request.method).c_str(), 1) !=
        0) {
      throw std::runtime_error(
          "Error: Failed to set REQUEST_METHOD environment variable.");
    }

    std::string contentType = getHeaderEnvValue("Content-Type");
    // if (contentType.empty()) {
    //   throw std::runtime_error("Error: Missing 'Content-Type' header.");
    // }
    if (setenv("CONTENT_TYPE", contentType.c_str(), 1) != 0) {
      throw std::runtime_error(
          "Error: Failed to set CONTENT_TYPE environment variable.");
    }

    std::string contentLength = getHeaderEnvValue("Content-Length");
    // if (contentLength.empty()) {
    //   throw std::runtime_error("Error: Missing 'Content-Length' header.");
    // }
    // if (setenv("CONTENT_LENGTH", contentLength.c_str(), 1) != 0) {
    //   throw std::runtime_error(
    //       "Error: Failed to set CONTENT_LENGTH environment variable.");
    // }

    std::string queryString = getQueryFields();
    if (setenv("QUERY_STRING", queryString.c_str(), 1) != 0) {
      throw std::runtime_error(
          "Error: Failed to set QUERY_STRING environment variable.");
    }

    if (setenv("SCRIPT_NAME", _request.uri.c_str(), 1) != 0) {
      throw std::runtime_error(
          "Error: Failed to set SCRIPT_NAME environment variable.");
    }

    if (setenv("SERVER_PROTOCOL", _request.protocol_version.c_str(), 1) != 0) {
      throw std::runtime_error(
          "Error: Failed to set SERVER_PROTOCOL environment variable.");
    }

    std::string cookies = fetchCookies();
    if (!cookies.empty()) {
      if (setenv("HTTP_COOKIE", cookies.c_str(), 1) != 0) {
        throw std::runtime_error(
            "Error: Failed to set HTTP_COOKIE environment variable.");
      }
    }

  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    throw;
  }
}

std::string CGI::executeCGI(const std::string &scriptPath) {
  int pipeIn[2];
  int pipeOut[2];
  std::string cgiOutput;

  if (pipe(pipeIn) == -1 || pipe(pipeOut) == -1)
    throw std::runtime_error("Pipe creation failed");

  pid_t pid = fork();
  if (pid == -1)
    throw std::runtime_error("Fork failed");

  if (pid == 0) {
    dup2(pipeIn[0], STDIN_FILENO);
    dup2(pipeOut[1], STDOUT_FILENO);
    close(pipeIn[1]);
    close(pipeOut[0]);
    setCGIEnv();
    char *argv[] = {const_cast<char *>(scriptPath.c_str()), NULL};
    std::cerr << "path: " << scriptPath << std::endl; //TESTE
    if (execve(scriptPath.c_str(), argv, environ) == -1)
      throw std::runtime_error("Exec failed");
  } else {
    close(pipeIn[0]);
    close(pipeOut[1]);

    time_t startTime = time(NULL);
    pidStartTimeMap[pid] = startTime;

    if (!_request.message_body.empty())
      write(pipeIn[1], _request.message_body.c_str(),
            _request.message_body.size());
    close(pipeIn[1]);

    char buffer[1024];
    ssize_t bytesRead;

    fd_set readFds;
    struct timeval timeout;

    while (true) {
      timeout.tv_sec = 0;
      timeout.tv_usec = 5000000;

      FD_ZERO(&readFds);
      FD_SET(pipeOut[0], &readFds);

      int activity = select(pipeOut[0] + 1, &readFds, NULL, NULL, &timeout);

      if (activity < 0) {
        if (errno == EINTR)
          std::cerr << "Select call interrupted by a singnal." << std::endl;
        else
          std::cerr << "Select error: " << strerror(errno) << std::endl;
      } else if (activity == 0) {
        // Timeout occurred
        std::cout << "Select time out." << std::endl;
      } else if (activity > 0) {
        bytesRead = read(pipeOut[0], buffer, sizeof(buffer));
        if (bytesRead > 0)
          cgiOutput.append(buffer, bytesRead);
      }

      time_t currentTime = time(NULL);

      if (currentTime - pidStartTimeMap[pid] > 5) {
        std::cerr << "CGI script taking too long, killing process. \n";
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
        pidStartTimeMap.erase(pid);
        throw std::runtime_error("CGI execution timed out");
      }

      int status;
      pid_t result = waitpid(pid, &status, WNOHANG);

      if (result == pid) {
        pidStartTimeMap.erase(pid);
        break;
      }
    }
  }
  close(pipeOut[0]);

  return cgiOutput;
}

std::string CGI::getCGIScriptPath() {
  std::string basePath =
      "/home/tiaferna/42_Projects/Webserv/resources"; // Para discutir com a equipa se isto faz sentido!
  std::string scriptName = _request.uri;

  return basePath + scriptName;
}

std::multimap<std::string, std::string>
CGI::parseCGIHeaders(const std::string &headers) {
  std::multimap<std::string, std::string> headerEnv;
  std::istringstream stream(headers);
  std::string line;

  while (std::getline(stream, line)) {
    size_t colonPos = line.find(": ");
    if (colonPos != std::string::npos) {
      std::string key = line.substr(0, colonPos);
      std::string value = line.substr(colonPos + 2);
      headerEnv.insert(std::make_pair(key, value));
    }
  }
  return headerEnv;
}

std::multimap<std::string, std::string> CGI::parseRequestHeaders() {
  std::multimap<std::string, std::string> headerEnv;
  for (std::multimap<std::string, std::string>::const_iterator it =
           _request.header_fields.begin();
       it != _request.header_fields.end(); ++it) {
    std::string key = it->first;
    std::string value = it->second;

    // if (key == "Content-Type" && value.empty()) {
    //   throw std::runtime_error("Error: Missing 'Content-Type' header.");
    // }
    // if (key == "Content-Length" && value.empty()) {
    //   throw std::runtime_error("Error: Missing 'Content-Length' header.");
    // }

    headerEnv.insert(std::make_pair(key, value));
  }
  return headerEnv;
}

std::string CGI::handleCGIResponse() {
  std::string scriptPath = getCGIScriptPath();
  std::string cgiOutput = executeCGI(scriptPath);
std::cout << "cgiOutput: " << cgiOutput << " _" << std::endl; //TESTE
  size_t pos = cgiOutput.find("\r\n\r\n");
  // if (pos != std::string::npos) {
    std::string headers = cgiOutput.substr(0, pos);
    std::string message = "HTTP/1.1 200 OK\r\n" + cgiOutput;
    std::multimap<std::string, std::string> headerEnv =
        parseCGIHeaders(headers);

    // if (headerEnv.find("Content-Type") == headerEnv.end()) {
    //   throw std::runtime_error("Missing 'Content-Type' in CGI headers.");
    // }
  
    return message;
  // } else {
  //   throw std::runtime_error(
  //       "Error: Malformed CGI output, no valid headers found.");
  // }

  // return "400 - DEU MERDA";
}
