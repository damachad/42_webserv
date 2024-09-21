#include "CGI.hpp"

CGI::CGI(HTTP_Request &httpRequest) : _request(httpRequest) {}

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
	// Find the range of values associated with the key
	std::pair<std::multimap<std::string, std::string>::iterator,
			  std::multimap<std::string, std::string>::iterator>
		range;
	range = _request.header_fields.equal_range(key);

	if (isSingleValueHeader(key)) {
		// If there are values associated with the key, return the first one
		if (range.first != range.second) {
			return range.first->second;	 // Return the first matching value
		}
	} else {
		std::string result;
		for (std::multimap<std::string, std::string>::iterator it = range.first;
			 it != range.second; ++it) {
			if (!result.empty()) {
				result += ", ";
			}
			result += it->second;
		}

		return result;
	}
	return "";	// Return an empty string if the key is not found
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
	setenv("REQUEST_METHOD", intToString(_request.method).c_str(), 1);
	setenv("CONTENT_TYPE", getHeaderEnvValue("Content-Type").c_str(), 1);
	setenv("CONTENT_LENGTH", getHeaderEnvValue("Content-Length").c_str(), 1);
	setenv("QUERY_STRING", getQueryFields().c_str(), 1);
	setenv("SCRIPT_NAME", _request.uri.c_str(), 1);
	setenv("SERVER_PROTOCOL", _request.protocol_version.c_str(), 1);
	setenv("HTTP_COOKIE", fetchCookies().c_str(), 1);
	// Adicionar ou reomver de acordo com os requesitios do nosso server
}

std::string CGI::executeCGI(const std::string &scriptPath) {
	int pipeIn[2];
	int pipeOut[2];
	std::string cgiOutput;

	if (pipe(pipeIn) == -1 || pipe(pipeOut) == -1)
		throw std::runtime_error("Pipe creation failed");

	pid_t pid = fork();
	if (pid == -1) throw std::runtime_error("Fork failed");

	if (pid == 0) {
		dup2(pipeIn[0], STDIN_FILENO);
		dup2(pipeOut[1], STDOUT_FILENO);
		close(pipeIn[1]);
		close(pipeOut[0]);
		setCGIEnv();
		char *argv[] = {const_cast<char *>(scriptPath.c_str()), NULL};
		if (execve(scriptPath.c_str(), argv, environ) == -1)
			throw std::runtime_error("Exec failed");
	} else {
		close(pipeIn[0]);
		close(pipeOut[1]);
		if (!_request.message_body.empty())
			write(pipeIn[1], _request.message_body.c_str(),
				  _request.message_body.size());
		close(pipeIn[1]);

		char buffer[1024];
		ssize_t bytesRead;
		while ((bytesRead = read(pipeOut[0], buffer, sizeof(buffer))) > 0)
			cgiOutput.append(buffer, bytesRead);
	}
	close(pipeOut[0]);

	waitpid(pid, NULL, 0);

	return cgiOutput;
}

std::string CGI::getCGIScriptPath() {
	std::string basePath = "/var/www/cgi-bin/";	 // para ser discutido em grupo
	std::string scriptName = _request.uri;

	return basePath + scriptName;
}

void CGI::handleCGIResponse() {
	std::string getCGIScriptPath();
	std::string cgiOutput = executeCGI();
}
