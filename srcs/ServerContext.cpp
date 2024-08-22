/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerContext.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 11:47:36 by damachad          #+#    #+#             */
/*   Updated: 2024/08/22 17:08:55 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

ServerContext::ServerContext() : _autoIndex(FALSE), _clientMaxBodySize(1048576) {
	// NGINX defaults
	_index.push_back("index.html");
	_index.push_back("index.htm");
	initializeDirectiveMap();
	std::vector<Method> methods;
	methods.push_back(GET);
	methods.push_back(POST);
	methods.push_back(DELETE);
	_allowedMethods = methods;
}

ServerContext::ServerContext(const ServerContext & src) : \
_network_address(src.getNetworkAddress()), \
_serverName(src.getServerName()), \
_root(src.getRoot()), \
_index(src.getIndex()), \
_autoIndex(src.getAutoIndex()), \
_clientMaxBodySize(src.getClientMaxBodySize()), \
_tryFiles(src.getTryFiles()), \
_allowedMethods(src.getAllowedMethods()), \
_errorPages(src.getErrorPages()), \
_locations(src.getLocations()) {}

ServerContext & ServerContext::operator=(const ServerContext & src) {
	_network_address = src.getNetworkAddress();
	_serverName = src.getServerName();
	_root = src.getRoot();
	_index = src.getIndex();
	_autoIndex = src.getAutoIndex();
	_clientMaxBodySize = src.getClientMaxBodySize();
	_tryFiles = src.getTryFiles();
	_allowedMethods = src.getAllowedMethods();
	_errorPages = src.getErrorPages();
	_locations = src.getLocations();
	return (*this);
}

ServerContext::~ServerContext() {}

// Function to initialize the map
void ServerContext::initializeDirectiveMap(void) {
	_directiveMap["listen"] = &ServerContext::handleListen;
	_directiveMap["server_name"] = &ServerContext::handleServerName;
	_directiveMap["root"] = &ServerContext::handleRoot;
	_directiveMap["index"] = &ServerContext::handleIndex;
	_directiveMap["try_files"] = &ServerContext::handleTryFiles;
	_directiveMap["error_page"] = &ServerContext::handleErrorPage;
	_directiveMap["client_max_body_size"] = &ServerContext::handleCliMaxSize;
	_directiveMap["autoindex"] = &ServerContext::handleAutoIndex;
	// _directiveMap["redirect"] = &ServerContext::handleRedirect;
}

static bool isValidPort(const std::string& port) {
	for (std::string::size_type i = 0; i < port.size(); ++i) {
		if (!std::isdigit(port[i])) return false;
	}
	int portNumber = std::atoi(port.c_str());
	return (portNumber >= 0 && portNumber <= 65535);
}

static bool isValidIp(const std::string& ip) {
	if (ip.empty() || ip == "localhost") return true;
	int numDots = 0;
	std::string segment;
	std::istringstream ipStream(ip);

	while (std::getline(ipStream, segment, '.'))
	{
		if (segment.empty() || segment.size() > 3) return false;
		for (std::string::size_type i = 0; i < segment.size(); i++)
			if (!std::isdigit(segment[i])) return false;
		int num = std::atoi(segment.c_str());
		if (num < 0 || num > 255) return false;
		numDots++;
	}
	return (numDots == 4);
}

//Handlers

// TODO: implement IPv6? default_server ?
// test what happens in NGINX address:<nothing> or <nothing>:port, 
// is it the same as not including that parameter?
void ServerContext::handleListen(std::vector<std::string> &tokens) {
	if (tokens.size() > 2)
		throw ConfigError("Too many arguments in listen directive.");
	std::vector<std::string>::const_iterator it; // check if accept just one token (if not implementing default_server)
	for (it = tokens.begin() + 1; it != tokens.end(); it++){
		Listen listening;
		std::string value = (*it);
		size_t colonPos = value.find(':');
		if (colonPos != std::string::npos) {
			listening.IP = value.substr(0, colonPos);
			listening.port = value.substr(colonPos + 1);
			if (listening.IP.empty() || listening.port.empty())
				throw ConfigError("Invalid IP:Port.");
		} else {
			if ((*it).find_first_not_of("0123456789") == std::string::npos)
				listening.port = value;
			else
				listening.IP = value;
		}
		if (listening.IP.empty())
			listening.IP = ""; // default
		if (listening.port.empty())
			listening.port = "80"; // default
		if (!isValidIp(listening.IP) || !isValidPort(listening.port))
			throw ConfigError("Invalid IP:Port.");
		_network_address.push_back(listening);
	}
}

void ServerContext::handleServerName(std::vector<std::string> &tokens) {
	tokens.erase(tokens.begin());
	_serverName = tokens;
}

void ServerContext::handleRoot(std::vector<std::string> &tokens) {
	if (tokens.size() > 2)
		throw ConfigError("Too many arguments in root directive.");
	_root = tokens[1];
}

void ServerContext::handleIndex(std::vector<std::string> &tokens) {
	tokens.erase(tokens.begin());
	_index = tokens;
}

void ServerContext::handleTryFiles(std::vector<std::string> &tokens) {
	tokens.erase(tokens.begin());
	_tryFiles = tokens;
}

void ServerContext::handleErrorPage(std::vector<std::string> &tokens) {
	if (tokens.size() < 3)
		throw ConfigError("Invalid error_page directive.");
	std::string page = tokens.back();
	for (size_t i = 1; i < tokens.size() - 1; i++){
		char *end;
		long statusCodeLong = std::strtol(tokens[i].c_str(), &end, 10);
		if (*end != '\0' || statusCodeLong < 100 || statusCodeLong > 599 \
		|| statusCodeLong != static_cast<short>(statusCodeLong))
				throw ConfigError("Invalid status code in error_page directive.");
		_errorPages[static_cast<short>(statusCodeLong)] = page;
	}
}

void ServerContext::handleCliMaxSize(std::vector<std::string> &tokens) {
	if (tokens.size() != 2)
		throw ConfigError("Invalid client_max_body_size directive.");
	std::string maxSize = tokens[1];
	char unit = maxSize[maxSize.size() - 1];
	maxSize.resize(maxSize.size() - 1);
	
	// check if there is overflow
	char *endPtr = NULL;
	long size = std::strtol(maxSize.c_str(), &endPtr, 10);
	if (*endPtr != '\0')
		throw ConfigError("Invalid numeric value for client_max_body_size.");
	// check for overflow during multiplication
	const long maxLimit = LONG_MAX;
	switch (unit)
	{
	case 'b':
	case 'B':
		_clientMaxBodySize = size;
		break;
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

void ServerContext::handleAutoIndex(std::vector<std::string> &tokens) {
	_autoIndex = FALSE; // review
	if (tokens[1] == "on")
		_autoIndex = TRUE;
	else if (tokens[1] != "off")
		throw ConfigError("Invalid syntax.");
}

void ServerContext::processDirective(std::string &line) {
	std::vector<std::string> tokens;
	tokens = ConfigParser::tokenizeLine(line);
	if (tokens.size() < 2)
		throw ConfigError("No value for directive: " + tokens[0]);
	std::map<std::string, DirectiveHandler>::const_iterator it;
	it = _directiveMap.find(tokens[0]);
	if (it != _directiveMap.end())
		(this->*(it->second))(tokens);
	else
		throw ConfigError("Unkown directive: " + tokens[0]);
}

// TODO: improve flow of function
//		load inherited values?
void ServerContext::processLocation(std::string block, size_t start, size_t end) {
	std::string route;
	std::string line;
	std::string firstWord;
	std::vector<std::string> tokens;
	std::istringstream location(block.substr(start, end - start));
	LocationContext locationInfo;

	location >> route;  				// Discard 'location'
	location >> route;  				// Get the actual route
	if (route == "{")
		throw ConfigError("No location route.");
	location >> line;  // Discard the opening '{'
	if (line != "{")
		throw ConfigError("Location can only support one route.");
	while (std::getline(location, line, ';'))
	{
		ConfigParser::trimOuterSpaces(line);
		if ((line.empty() && !location.eof()))
			throw ConfigError("Unparsable location block detected.");
		if (line.empty())
			continue;
		std::istringstream readLine(line);
		readLine >> firstWord;
		locationInfo.processDirective(line);
		if (location.tellg() >= static_cast<std::streampos>(end)) 
			break;
	}
	_locations[route] = locationInfo;
}

// Getters
std::vector<Listen> ServerContext::getNetworkAddress() const {
	return _network_address;
}

std::vector<std::string> ServerContext::getServerName() const {
	return _serverName;
}

std::string ServerContext::getRoot() const {
	return _root;
}

std::vector<std::string> ServerContext::getIndex() const {
	return _index;
}

State ServerContext::getAutoIndex() const {
	return _autoIndex;
}

long ServerContext::getClientMaxBodySize() const {
	return _clientMaxBodySize;
}

std::vector<std::string> ServerContext::getTryFiles() const {
	return _tryFiles;
}

std::vector<Method> ServerContext::getAllowedMethods() const {
	return _allowedMethods;
}

std::map<short, std::string> ServerContext::getErrorPages() const {
	return _errorPages;
}

std::map<std::string, LocationContext> ServerContext::getLocations() const {
	return _locations;
}

std::string ServerContext::getRoot(const std::string &route) const {
	std::map<std::string, LocationContext>::const_iterator it;
	it = _locations.find(route);
	if (it == _locations.end() || it->second.getRoot().empty())
		return _root;
	else
		return it->second.getRoot();
}

std::vector<std::string> ServerContext::getIndex(const std::string &route) const {
	std::map<std::string, LocationContext>::const_iterator it;
	it = _locations.find(route);
	if (it == _locations.end() || it->second.getIndex().empty())
		return _index;
	else
		return it->second.getIndex();
}

State ServerContext::getAutoIndex(const std::string &route) const {
	std::map<std::string, LocationContext>::const_iterator it;
	it = _locations.find(route);
	if (it == _locations.end() || it->second.getAutoIndex() == UNSET)
		return _autoIndex;
	else
		return it->second.getAutoIndex();
}

long ServerContext::getClientMaxBodySize(const std::string &route) const {
	std::map<std::string, LocationContext>::const_iterator it;
	it = _locations.find(route);
	if (it == _locations.end() || it->second.getClientMaxBodySize() == -1)
		return _clientMaxBodySize;
	else
		return it->second.getClientMaxBodySize();
}

std::vector<std::string> ServerContext::getTryFiles(const std::string &route) const {
	std::map<std::string, LocationContext>::const_iterator it;
	it = _locations.find(route);
	if (it == _locations.end() || it->second.getTryFiles().empty())
		return _tryFiles;
	else
		return it->second.getTryFiles();
}

std::map<short, std::string> ServerContext::getErrorPages(const std::string &route) const {
	std::map<std::string, LocationContext>::const_iterator it;
	it = _locations.find(route);
	if (it == _locations.end() || it->second.getErrorPages().empty())
		return _errorPages;
	else
		return it->second.getErrorPages();
}

std::vector<Method> ServerContext::getAllowedMethods(const std::string &route) const {
	
	std::map<std::string, LocationContext>::const_iterator it;
	it = _locations.find(route);
	if (it == _locations.end() || it->second.getAllowedMethods().empty())
		return _allowedMethods;
	else
		return it->second.getAllowedMethods();
}

std::ostream& operator<<(std::ostream& os, const ServerContext& context) {
	os << "Network Addresses:\n";
	std::vector<Listen> networkAddress = context.getNetworkAddress();
	for (std::vector<Listen>::const_iterator it = networkAddress.begin(); it != networkAddress.end(); ++it)
		os << "  IP: " << it->IP << ", Port: " << it->port << "\n";

	os << "Server Names:\n";
	std::vector<std::string> serverNames = context.getServerName();
	for (std::vector<std::string>::const_iterator it = serverNames.begin(); it != serverNames.end(); ++it)
		os << "  " << *it << "\n";

	os << "Root: " << context.getRoot() << "\n";

	os << "Index Files:\n";
	std::vector<std::string> indexFiles = context.getIndex();
	for (std::vector<std::string>::const_iterator it = indexFiles.begin(); it != indexFiles.end(); ++it) {
		os << "  " << *it << "\n";
	}

	os << "AutoIndex: " << (context.getAutoIndex() == TRUE ? "TRUE" : context.getAutoIndex() == FALSE ? "FALSE" : "UNSET") << "\n";

	os << "Client Max Body Size: " << context.getClientMaxBodySize() << "\n";

	os << "Try Files:\n";
	std::vector<std::string> tryFiles = context.getTryFiles();
	for (std::vector<std::string>::const_iterator it = tryFiles.begin(); it != tryFiles.end(); ++it) {
		os << "  " << *it << "\n";
	}

	os << "Error Pages:\n";
	std::map<short, std::string> errorPages = context.getErrorPages();
	for (std::map<short, std::string>::const_iterator it = errorPages.begin(); it != errorPages.end(); ++it) {
		os << "  " << it->first << " : " << it->second << "\n";
	}

	os << "Locations:\n";
	std::map<std::string, LocationContext> locations = context.getLocations();
	for (std::map<std::string, LocationContext>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
		os << "  " << it->first << " :\n" << it->second << "\n";
	}

	return os;
}
