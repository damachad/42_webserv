#include "CGI.hpp"

#include "AResponse.hpp"

CGI::CGI(HttpRequest &httpRequest, HTTP_Response &httpResponse,
		 const std::string &path)
	: _request(httpRequest),
	  _response(httpResponse),
	  _path(path),
	  _cgiEnv(NULL) {}

CGI::~CGI() {
	if (_cgiEnv) {
		for (size_t i = 0; _cgiEnv[i] != NULL; ++i) {
			delete[] _cgiEnv[i];
		}
		delete[] _cgiEnv;
	}
}

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
		if (it->first == "cookie") {
			if (!result.empty()) {
				result += "; ";
			}
			result += it->second;
		}
	}
	return result;
}

char **CGI::vectorToCharArray(const std::vector<std::string> &vec) {
	char **charArray = new char *[vec.size() + 1];

	for (size_t i = 0; i < vec.size(); ++i) {
		charArray[i] = new char[vec[i].size() + 1];
		std::strcpy(charArray[i], vec[i].c_str());
	}

	charArray[vec.size()] = NULL;

	return charArray;
}

void CGI::setSingleEnv(std::vector<std::string> &env, std::string key,
					   std::string envToAdd) {
	env.push_back(key + "=" + envToAdd);
}

std::string CGI::getServerName() {
	std::multimap<std::string, std::string>::const_iterator it =
		_request.header_fields.find("host");

	if (it == _request.header_fields.end()) {
		return "";
	}

	std::string hostName = it->second;

	size_t colonPos = hostName.find_first_of(':');
	if (colonPos != std::string::npos) hostName = hostName.substr(0, colonPos);

	return hostName;
}

std::string CGI::getServerPort() {
	std::multimap<std::string, std::string>::const_iterator it =
		_request.header_fields.find("host");

	if (it == _request.header_fields.end()) {
		return "";
	}

	std::string hostName = it->second;

	size_t colonPos = hostName.find_first_of(':');
	if (colonPos != std::string::npos) hostName = hostName.substr(colonPos + 1);

	return hostName;
}

short CGI::setCGIEnv() {
	if (_request.method == POST && getHeaderEnvValue("content-type").empty())

		return 500;
	if (_request.method == POST && getHeaderEnvValue("content-length").empty())
		return 500;

	std::vector<std::string> cgiEnvironments;

	setSingleEnv(cgiEnvironments, "REQUEST_METHOD",
				 methodToString(_request.method).c_str());
	setSingleEnv(cgiEnvironments, "CONTENT_TYPE",
				 getHeaderEnvValue("content-type").c_str());
	setSingleEnv(cgiEnvironments, "SERVER_NAME", getServerName().c_str());
	setSingleEnv(cgiEnvironments, "SERVER_PORT", getServerPort().c_str());
	setSingleEnv(cgiEnvironments, "GATEWAY_INTERFACE", "CGI/1.1");
	setSingleEnv(cgiEnvironments, "PATH_INFO", _path.c_str());
	setSingleEnv(cgiEnvironments, "CONTENT_LENGTH",
				 getHeaderEnvValue("content-length").c_str());
	setSingleEnv(cgiEnvironments, "QUERY_STRING", getQueryFields().c_str());
	setSingleEnv(cgiEnvironments, "SCRIPT_NAME", _request.uri.c_str());
	setSingleEnv(cgiEnvironments, "SERVER_PROTOCOL",
				 _request.protocol_version.c_str());
	setSingleEnv(cgiEnvironments, "SERVER_SOFTWARE", SERVER);

	std::string cookies = fetchCookies();
	if (!cookies.empty())
		setSingleEnv(cgiEnvironments, "HTTP_COOKIE", cookies.c_str());

	_cgiEnv = vectorToCharArray(cgiEnvironments);

	return 200;
}

std::string CGI::createCgiOutput(pid_t pid, int *pipeOut) {
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

	pid_t pid = fork();
	if (pid == -1) return "500";

	if (pid == 0) {
		dup2(pipeIn[0], STDIN_FILENO);
		dup2(pipeOut[1], STDOUT_FILENO);
		close(pipeIn[1]);
		close(pipeOut[0]);
		short status = setCGIEnv();
		if (status != 200) return numberToString(status);
		setLimits(MEMORYCHILD);
		std::string dirName =
			scriptPath.substr(0, scriptPath.find_last_of("/"));
		if (chdir(dirName.c_str()) < 0) exit(1);

		char *argv[] = {const_cast<char *>(scriptPath.c_str()), NULL};
		if (execve(scriptPath.c_str(), argv, _cgiEnv) == -1) exit(1);

	} else {
		close(pipeIn[0]);
		close(pipeOut[1]);
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
		std::multimap<std::string, std::string> headerEnv =
			parseCGIHeaders(headers);

		// Preserve existing headers if necessary
		for (std::multimap<std::string, std::string>::const_iterator it =
				 headerEnv.begin();
			 it != headerEnv.end(); ++it) {
			// Check if the header is already set
			std::multimap<std::string, std::string>::iterator responseIt =
				_response.headers.find(it->first);
			if (responseIt == _response.headers.end()) {
				// If the header is not already present, insert it
				_response.headers.insert(*it);
			}
		}
		_response.body = body;
	} else
		_response.status = 500;
}
