/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 11:47:36 by damachad          #+#    #+#             */
/*   Updated: 2024/10/16 13:57:40 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"

Location::Location() : _auto_index(UNSET), _client_max_body_size(-1) {
	initializeDirectiveMap();
	_return = std::make_pair(-1, "");
}

Location::Location(const Location &src)
	: _root(src.getRoot()),
	  _index(src.getIndex()),
	  _auto_index(src.getAutoIndex()),
	  _client_max_body_size(src.getClientMaxBodySize()),
	  _allowed_methods(src.getAllowedMethods()),
	  _error_pages(src.getErrorPages()),
	  _return(src.getReturn()),
	  _upload_store(src.getUpload()),
	  _cgi_ext(src.getCgiExt()) {}

Location &Location::operator=(const Location &src) {
	_root = src.getRoot();
	_index = src.getIndex();
	_auto_index = src.getAutoIndex();
	_client_max_body_size = src.getClientMaxBodySize();
	_allowed_methods = src.getAllowedMethods();
	_error_pages = src.getErrorPages();
	_return = src.getReturn();
	_upload_store = src.getUpload();
	_cgi_ext = src.getCgiExt();
	return (*this);
}

Location::~Location() {}

// Function to initialize the map
void Location::initializeDirectiveMap(void) {
	_directive_map["root"] = &Location::handleRoot;
	_directive_map["index"] = &Location::handleIndex;
	_directive_map["limit_except"] = &Location::handleLimitExcept;
	_directive_map["error_page"] = &Location::handleErrorPage;
	_directive_map["client_max_body_size"] = &Location::handleCliMaxSize;
	_directive_map["autoindex"] = &Location::handleAutoIndex;
	_directive_map["return"] = &Location::handleReturn;
	_directive_map["upload_store"] = &Location::handleUpload;
	_directive_map["cgi_ext"] = &Location::handleCgiExt;
}

// Handlers

void Location::handleRoot(std::vector<std::string> &tokens) {
	if (tokens.size() > 2)
		throw ConfigError("Invalid number of arguments in root directive.");
	_root = tokens[1];
}

void Location::handleIndex(std::vector<std::string> &tokens) {
	tokens.erase(tokens.begin());
	_index = tokens;
}

void Location::handleLimitExcept(std::vector<std::string> &tokens) {
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
	_allowed_methods = methods;
}

void Location::handleErrorPage(std::vector<std::string> &tokens) {
	if (tokens.size() < 3)
		throw ConfigError(
			"Invalid number of arguments in error_page directive.");
	std::string page = tokens.back();
	for (size_t i = 1; i < tokens.size() - 1; i++) {
		char *end;
		long statusCodeLong = std::strtol(tokens[i].c_str(), &end, 10);
		if (*end != '\0' || statusCodeLong < 300 || statusCodeLong > 599 ||
			statusCodeLong != static_cast<short>(statusCodeLong))
			throw ConfigError("Invalid status code in error_page directive.");
		_error_pages[static_cast<short>(statusCodeLong)] = page;
	}
}

void Location::handleCliMaxSize(std::vector<std::string> &tokens) {
	if (tokens.size() != 2)
		throw ConfigError(
			"Invalid number of arguments in client_max_body_size directive.");
	std::string maxSize = tokens[1];
	char unit = maxSize[maxSize.size() - 1];
	if (!std::isdigit(unit)) maxSize.resize(maxSize.size() - 1);

	// check if there is overflow
	char *endPtr = NULL;
	long size = std::strtol(maxSize.c_str(), &endPtr, 10);
	if (*endPtr != '\0')
		throw ConfigError("Invalid value for client_max_body_size.");
	_client_max_body_size = size;
	if (!std::isdigit(unit)) {
		// check for overflow during multiplication
		const long maxLimit = LONG_MAX;
		switch (unit) {
			case 'k':
			case 'K':
				if (size > maxLimit / 1024)
					throw ConfigError("client_max_body_size value overflow.");
				_client_max_body_size = size * 1024;
				break;
			case 'm':
			case 'M':
				if (size > maxLimit / 1048576)
					throw ConfigError("client_max_body_size value overflow.");
				_client_max_body_size = size * 1048576;
				break;
			case 'g':
			case 'G':
				if (size > maxLimit / 1073741824)
					throw ConfigError("client_max_body_size value overflow.");
				_client_max_body_size = size * 1073741824;
				break;
			default:
				throw ConfigError("Invalid unit for client_max_body_size.");
		}
	}
}

void Location::handleAutoIndex(std::vector<std::string> &tokens) {
	if (tokens[1] == "on")
		_auto_index = TRUE;
	else if (tokens[1] == "off")
		_auto_index = FALSE;
	else
		throw ConfigError("Invalid value in autoindex directive.");
}

void Location::handleReturn(std::vector<std::string> &tokens) {
	if (tokens.size() != 3)
		throw ConfigError("Invalid number of arguments in return directive.");
	char *endPtr = NULL;
	long errorCode = std::strtol(tokens[1].c_str(), &endPtr, 10);
	if (*endPtr != '\0' || errorCode < 0 || errorCode > 999 ||
		errorCode != static_cast<short>(errorCode))	 // accepted NGINX values
		throw ConfigError("Invalid error code for return directive.");
	if (_return.first != -1) return;
	_return.first = static_cast<short>(errorCode);
	_return.second = tokens[2];
}

void Location::handleUpload(std::vector<std::string> &tokens) {
	if (tokens.size() != 2)
		throw ConfigError(
			"Invalid number of arguments in upload_store directive.");
	_upload_store = tokens[1];
}

void Location::handleCgiExt(std::vector<std::string> &tokens) {
	if (tokens.size() > 2)
		throw ConfigError("Invalid number of arguments in cgi_ext directive.");
	_cgi_ext = tokens[1];
}

void Location::processDirective(std::string &line) {
	std::vector<std::string> tokens;
	tokens = ConfigParser::tokenizeLine(line);
	if (tokens.size() < 2)
		throw ConfigError("Invalid number of arguments in \"" + tokens[0] +
						  "\" directive.");
	std::map<std::string, DirectiveHandler>::const_iterator it;
	it = _directive_map.find(tokens[0]);
	if (it != _directive_map.end())
		(this->*(it->second))(tokens);
	else
		throw ConfigError("Unkown directive \"" + tokens[0] + "\"");
}

// Getters
std::string Location::getRoot() const { return _root; }

std::vector<std::string> Location::getIndex() const { return _index; }

State Location::getAutoIndex() const { return _auto_index; }

long Location::getClientMaxBodySize() const { return _client_max_body_size; }

std::set<Method> Location::getAllowedMethods() const {
	return _allowed_methods;
}

std::map<short, std::string> Location::getErrorPages() const {
	return _error_pages;
}

std::pair<short, std::string> Location::getReturn() const { return _return; }

std::string Location::getUpload() const { return _upload_store; }

std::string Location::getCgiExt() const { return _cgi_ext; }

std::ostream &operator<<(std::ostream &os, const Location &context) {
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
