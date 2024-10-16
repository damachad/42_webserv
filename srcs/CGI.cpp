#include "CGI.hpp"
#include "AResponse.hpp"

CGI::CGI(HTTP_Request &httpRequest, HTTP_Response &httpResponse,
		 const std::string &path)
	: _request(httpRequest), _response(httpResponse), _path(path) {}

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
		if (!queryString.empty()) queryString += "&";
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
		for (std::multimap<std::string, std::string>::const_iterator it =
				 range.first;
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

short CGI::setCGIEnv() {
	if (setenv("REQUEST_METHOD", intToString(_request.method).c_str(), 1) !=
		0) {
		return 500;
	}
	std::string contentType = getHeaderEnvValue("content-type");
	if (_request.method == POST && contentType.empty()) {
		return 500;
	}
	if (setenv("CONTENT_TYPE", contentType.c_str(), 1) != 0) {
		return 500;
	}
	if (setenv("PATH_INFO", _path.c_str(), 1) != 0) {
		return 500;
	}

	std::string contentLength = getHeaderEnvValue("content-length");
	if (_request.method == POST && contentLength.empty()) {
		return 500;
	}
	if (setenv("CONTENT_LENGTH", contentLength.c_str(), 1) != 0) {
		return 500;
	}

	std::string queryString = getQueryFields();
	if (setenv("QUERY_STRING", queryString.c_str(), 1) != 0) {
		return 500;
	}

	if (setenv("SCRIPT_NAME", _request.uri.c_str(), 1) != 0) {
		return 500;
	}

	if (setenv("SERVER_PROTOCOL", _request.protocol_version.c_str(), 1) != 0) {
		return 500;
	}

	std::string cookies = fetchCookies();
	if (!cookies.empty()) {
		if (setenv("HTTP_COOKIE", cookies.c_str(), 1) != 0) {
			return 500;
		}
	}
	return 200;
}

std::string readHtmlFile(const std::string &filePath) {
	std::ifstream file(filePath.c_str());
	if (!file.is_open()) {
		std::cerr << "Error: Could not open HTML file." << std::endl;
		return "";
	}

	std::stringstream buffer;
	buffer << file.rdbuf();	 // Read the entire file into the buffer
	return buffer.str();	 // Return the string containing HTML content
}

std::string createCgiOutput(pid_t pid, int *pipeOut) {
	std::string cgiOutput;
	char buffer[1024];
	ssize_t bytesRead;
	struct timeval startTime;
	struct timeval currentTime;

	gettimeofday(&startTime, NULL);

	while (true) {
		gettimeofday(&currentTime, NULL);
		int status;

		if (waitpid(pid, &status, WNOHANG) != 0) {
			if (WEXITSTATUS(status) != 0) return "500";
			while ((bytesRead = read(pipeOut[0], buffer, sizeof(buffer))) > 0)
				cgiOutput.append(buffer, bytesRead);
			break;
		}

		if (currentTime.tv_sec - startTime.tv_sec > TIMEOUT) {
			kill(pid, SIGKILL);
			return "504";
		}
	}
	return cgiOutput;
}

// Sets the maximum size of a process' total available memory
static void setLimits(int limitMb) {
	struct rlimit limit;

	// Set memory limit (limit MB)
	limit.rlim_cur = limitMb * 1024 * 1024;
	limit.rlim_max = limitMb * 1024 * 1024;
	setrlimit(RLIMIT_AS, &limit);
}

std::string CGI::executeCGI(const std::string &scriptPath) {
	int pipeIn[2];
	int pipeOut[2];
	std::string cgiOutput;

	if (pipe(pipeIn) == -1 || pipe(pipeOut) == -1) return "500";
				std::cout << "!!CAIU 1" << std::endl; // TESTE

	pid_t pid = fork();
	if (pid == -1) return "500";
				std::cout << "!!CAIU 2" << std::endl; // TESTE

	if (pid == 0) {
		dup2(pipeIn[0], STDIN_FILENO);
		dup2(pipeOut[1], STDOUT_FILENO);
		close(pipeIn[1]);
		close(pipeOut[0]);
		short status = setCGIEnv();
		std::cerr << "!!STATUS: " << status << std::endl; // TESTE
		if (status != 200) return numberToString(status);
		setLimits(MEMORYCHILD);
		std::string dirName =
			scriptPath.substr(0, scriptPath.find_last_of("/"));
			std::cerr << "!!DIR NAME: " << dirName << std::endl; // TESTE
		if (chdir(dirName.c_str()) < 0) return "500";
					std::cerr << "!!CAIU 3" << std::endl; // TESTE

		char *argv[] = {const_cast<char *>(scriptPath.c_str()), NULL};
		if (execve(scriptPath.c_str(), argv, environ) == -1) return "500";
					std::cerr << "!!CAIU 4" << std::endl; // TESTE

	} else {
		close(pipeIn[0]);
		close(pipeOut[1]);
		write(1, _request.message_body.c_str(),
				  _request.message_body.size()); //TESTE
		if (!_request.message_body.empty())
			write(pipeIn[1], _request.message_body.c_str(),
				  _request.message_body.size());
		close(pipeIn[1]);

		cgiOutput = createCgiOutput(pid, pipeOut);
	}
	close(pipeOut[0]);
	return cgiOutput;
}

std::multimap<std::string, std::string> CGI::parseCGIHeaders(
	const std::string &headers) {
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

		if (key == "Content-Type" && value.empty()) {
			std::cout << "!!CAIU 5" << std::endl; // TESTE
			_response.status = 500;
		}

		headerEnv.insert(std::make_pair(key, value));
	}
	return headerEnv;
}

void CGI::handleCGIResponse() {
    std::string cgiOutput = executeCGI(_path);
    
    // Check if the CGI output is an error status
    if (std::atoi(cgiOutput.c_str()) != 0) {
        _response.status = std::atoi(cgiOutput.c_str());
        return;
    }

    size_t pos = cgiOutput.find("\r\n\r\n");
    if (pos != std::string::npos) {
        std::string headers = cgiOutput.substr(0, pos);
        std::string body = cgiOutput.substr(pos + 4);
        std::multimap<std::string, std::string> headerEnv = parseCGIHeaders(headers);

        // Preserve existing headers if necessary
        for (std::multimap<std::string, std::string>::const_iterator it = headerEnv.begin(); 
             it != headerEnv.end(); ++it) {
            // Check if the header is already set
            std::multimap<std::string, std::string>::iterator responseIt = _response.headers.find(it->first);
            if (responseIt == _response.headers.end()) {
                // If the header is not already present, insert it
                _response.headers.insert(*it);
            } else {
                // If the header is present, log a message or handle as needed
                std::cout << "Header already exists: " << it->first << std::endl;
            }
        }

        _response.body = body;
        std::cout << "!!RESPONSE BODY: " << _response.body << std::endl;
    } else {
        std::cout << "!!CAIU 7" << std::endl; // TESTE
        _response.status = 500;
    }
}

