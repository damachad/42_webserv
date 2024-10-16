/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationContext.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 11:47:36 by damachad          #+#    #+#             */
/*   Updated: 2024/10/16 13:57:40 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationContext.hpp"

LocationContext::LocationContext() : _autoIndex(UNSET), _clientMaxBodySize(-1) {
	initializeDirectiveMap();
	_return = std::make_pair(-1, "");
}

LocationContext::LocationContext(const LocationContext &src)
	: _root(src.getRoot()),
	  _index(src.getIndex()),
	  _autoIndex(src.getAutoIndex()),
	  _clientMaxBodySize(src.getClientMaxBodySize()),
	  _allowedMethods(src.getAllowedMethods()),
	  _errorPages(src.getErrorPages()),
	  _return(src.getReturn()),
	  _uploadStore(src.getUpload()),
	  _cgiExt(src.getCgiExt()) {}

LocationContext &LocationContext::operator=(const LocationContext &src) {
	_root = src.getRoot();
	_index = src.getIndex();
	_autoIndex = src.getAutoIndex();
	_clientMaxBodySize = src.getClientMaxBodySize();
	_allowedMethods = src.getAllowedMethods();
	_errorPages = src.getErrorPages();
	_return = src.getReturn();
	_uploadStore = src.getUpload();
	_cgiExt = src.getCgiExt();
	return (*this);
}

LocationContext::~LocationContext() {}

// Function to initialize the map
void LocationContext::initializeDirectiveMap(void) {
	_directiveMap["root"] = &LocationContext::handleRoot;
	_directiveMap["index"] = &LocationContext::handleIndex;
	_directiveMap["limit_except"] = &LocationContext::handleLimitExcept;
	_directiveMap["error_page"] = &LocationContext::handleErrorPage;
	_directiveMap["client_max_body_size"] = &LocationContext::handleCliMaxSize;
	_directiveMap["autoindex"] = &LocationContext::handleAutoIndex;
	_directiveMap["return"] = &LocationContext::handleReturn;
	_directiveMap["upload_store"] = &LocationContext::handleUpload;
	_directiveMap["cgi_ext"] = &LocationContext::handleCgiExt;
}

// Handlers

void LocationContext::handleRoot(std::vector<std::string> &tokens) {
	if (tokens.size() > 2) throw ConfigError("Invalid number of arguments in root directive.");
	_root = tokens[1];
}

void LocationContext::handleIndex(std::vector<std::string> &tokens) {
	tokens.erase(tokens.begin());
	_index = tokens;
}

void LocationContext::handleLimitExcept(std::vector<std::string> &tokens) {
	std::set<Method> methods;
	std::vector<std::string>::const_iterator it;
	for (it = tokens.begin() + 1; it != tokens.end(); it++) {
		if ((*it) == "GET")
			methods.insert(GET);
		else if ((*it) == "DELETE")
			methods.insert(DELETE);
		else if ((*it) == "POST")
			methods.insert(POST);
		else
			throw ConfigError("Unsupported method detected.");
	}
	_allowedMethods = methods;
}

void LocationContext::handleErrorPage(std::vector<std::string> &tokens) {
	if (tokens.size() < 3) throw ConfigError("Invalid number of arguments in error_page directive.");
	std::string page = tokens.back();
	for (size_t i = 1; i < tokens.size() - 1; i++) {
		char *end;
		long statusCodeLong = std::strtol(tokens[i].c_str(), &end, 10);
		if (*end != '\0' || statusCodeLong < 300 || statusCodeLong > 599 ||
			statusCodeLong != static_cast<short>(statusCodeLong))
			throw ConfigError("Invalid status code in error_page directive.");
		_errorPages[static_cast<short>(statusCodeLong)] = page;
	}
}

void LocationContext::handleCliMaxSize(std::vector<std::string> &tokens) {
	if (tokens.size() != 2)
		throw ConfigError("Invalid number of arguments in client_max_body_size directive.");
	std::string maxSize = tokens[1];
	char unit = maxSize[maxSize.size() - 1];
	if (!std::isdigit(unit)) maxSize.resize(maxSize.size() - 1);

	// check if there is overflow
	char *endPtr = NULL;
	long size = std::strtol(maxSize.c_str(), &endPtr, 10);
	if (*endPtr != '\0')
		throw ConfigError("Invalid value for client_max_body_size.");
	_clientMaxBodySize = size;
	if (!std::isdigit(unit)) {
		// check for overflow during multiplication
		const long maxLimit = LONG_MAX;
		switch (unit) {
			case 'k':
			case 'K':
				if (size > maxLimit / 1024)
					throw ConfigError("client_max_body_size value overflow.");
				_clientMaxBodySize = size * 1024;
				break;
			case 'm':
			case 'M':
				if (size > maxLimit / 1048576)
					throw ConfigError("client_max_body_size value overflow.");
				_clientMaxBodySize = size * 1048576;
				break;
			case 'g':
			case 'G':
				if (size > maxLimit / 1073741824)
					throw ConfigError("client_max_body_size value overflow.");
				_clientMaxBodySize = size * 1073741824;
				break;
			default:
				throw ConfigError("Invalid unit for client_max_body_size.");
		}
	}
}

void LocationContext::handleAutoIndex(std::vector<std::string> &tokens) {
	if (tokens[1] == "on")
		_autoIndex = TRUE;
	else if (tokens[1] == "off")
		_autoIndex = FALSE;
	else
		throw ConfigError("Invalid value in autoindex directive.");
}

void LocationContext::handleReturn(std::vector<std::string> &tokens) {
	if (tokens.size() != 3) throw ConfigError("Invalid number of arguments in return directive.");
	char *endPtr = NULL;
	long errorCode = std::strtol(tokens[1].c_str(), &endPtr, 10);
	if (*endPtr != '\0' || errorCode < 0 || errorCode > 999 ||
		errorCode != static_cast<short>(errorCode))	 // accepted NGINX values
		throw ConfigError("Invalid error code for return directive.");
	if (_return.first != -1) return;
	_return.first = static_cast<short>(errorCode);
	_return.second = tokens[2];
}

void LocationContext::handleUpload(std::vector<std::string> &tokens) {
	if (tokens.size() != 2) throw ConfigError("Invalid number of arguments in upload_store directive.");
	_uploadStore = tokens[1];
}

void LocationContext::handleCgiExt(std::vector<std::string> &tokens) {
	if (tokens.size() > 2) throw ConfigError("Invalid number of arguments in cgi_ext directive.");
	_cgiExt = tokens[1];
}

void LocationContext::processDirective(std::string &line) {
	std::vector<std::string> tokens;
	tokens = ConfigParser::tokenizeLine(line);
	if (tokens.size() < 2)
		throw ConfigError("Invalid number of arguments in \"" + tokens[0] + "\" directive.");
	std::map<std::string, DirectiveHandler>::const_iterator it;
	it = _directiveMap.find(tokens[0]);
	if (it != _directiveMap.end())
		(this->*(it->second))(tokens);
	else
		throw ConfigError("Unkown directive \"" + tokens[0] + "\"");
}

// Getters
std::string LocationContext::getRoot() const { return _root; }

std::vector<std::string> LocationContext::getIndex() const { return _index; }

State LocationContext::getAutoIndex() const { return _autoIndex; }

long LocationContext::getClientMaxBodySize() const {
	return _clientMaxBodySize;
}

std::set<Method> LocationContext::getAllowedMethods() const {
	return _allowedMethods;
}

std::map<short, std::string> LocationContext::getErrorPages() const {
	return _errorPages;
}

std::pair<short, std::string> LocationContext::getReturn() const {
	return _return;
}

std::string LocationContext::getUpload() const {
	return _uploadStore;
}

std::string LocationContext::getCgiExt() const {
	return _cgiExt;
}

std::ostream &operator<<(std::ostream &os, const LocationContext &context) {
	os << "  Root: " << context.getRoot() << "\n";

	os << "  Index Files:\n";
	std::vector<std::string> indexFiles = context.getIndex();
	for (std::vector<std::string>::const_iterator it = indexFiles.begin();
		 it != indexFiles.end(); ++it) {
		os << "    " << *it << "\n";
	}

	os << "  AutoIndex: "
	   << (context.getAutoIndex() == TRUE	 ? "TRUE"
		   : context.getAutoIndex() == FALSE ? "FALSE"
											 : "UNSET")
	   << "\n";

	os << "  Client Max Body Size: " << context.getClientMaxBodySize() << "\n";

	os << "  Allowed Methods: ";
	std::set<Method> methods = context.getAllowedMethods();
	for (std::set<Method>::const_iterator it = methods.begin();
		 it != methods.end(); ++it) {
		switch (*it) {
			case GET:
				os << "GET ";
				break;
			case POST:
				os << "POST ";
				break;
			case DELETE:
				os << "DELETE ";
				break;
			default:
				os << "UNKNOWN ";
		}
	}
	os << "\n";

	os << "  Error Pages:\n";
	std::map<short, std::string> errorPages = context.getErrorPages();
	for (std::map<short, std::string>::const_iterator it = errorPages.begin();
		 it != errorPages.end(); ++it) {
		os << "    " << it->first << " : " << it->second << "\n";
	}

	os << "  Return:\n";
	std::pair<short, std::string> returns = context.getReturn();
	if (returns.first != -1)
		os << "    " << returns.first << " : " << returns.second << "\n";
	
	os << "  Upload Store: " << context.getUpload() << "\n";
	os << "  CGI Extension: " << context.getCgiExt() << "\n";

	return os;
}
