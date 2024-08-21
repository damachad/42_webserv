/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/13 11:25:49 by damachad          #+#    #+#             */
/*   Updated: 2024/08/21 11:22:11 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

ConfigParser::ConfigParser(void) {}

ConfigParser::ConfigParser(const ConfigParser &src) { *this = src; }

ConfigParser::ConfigParser(const std::string &file) : _configFile(file) {
	initializeDirectiveMap();
}

ConfigParser::~ConfigParser(void) {}

ConfigParser &ConfigParser::operator=(const ConfigParser &src) {
	if (this != &src) {
		this->_configFile = src._configFile;
		this->_servers = src._servers;
	}
	return (*this);
}

void ConfigParser::loadDefaults(Context &context) {
	// default values from NGINX
	context.autoIndex = FALSE;
	context.clientMaxBodySize = 1048576; // 1m
	context.index.push_back("index.html");
	context.index.push_back("index.htm");
}

/* remove leading and trailing whitespaces */
void ConfigParser::trimOuterSpaces(std::string &s) {
	if (s.empty()) return;
	while (std::isspace(s[0])) s.erase(s.begin());
	if (s.empty())	// case where line only has WS
		return;
	while (std::isspace(s[s.length() - 1])) s.erase(s.end() - 1);
}

/* remove comments */
void ConfigParser::trimComments(std::string &s) {
	if (s.empty()) return;
	size_t comment = s.find('#');
	while (comment != std::string::npos) {
		size_t endl = s.find('\n', comment);
		s.erase(comment, endl - comment);
		comment = s.find('#');
	}
}

// Function to initialize the map
void ConfigParser::initializeDirectiveMap(void) {
	_directiveMap["listen"] = &ConfigParser::handleListen;
	_directiveMap["server_name"] = &ConfigParser::handleServerName;
	_directiveMap["root"] = &ConfigParser::handleRoot;
	_directiveMap["index"] = &ConfigParser::handleIndex;
	_directiveMap["limit_except"] = &ConfigParser::handleLimitExcept;
	_directiveMap["try_files"] = &ConfigParser::handleTryFiles;
	_directiveMap["error_page"] = &ConfigParser::handleErrorPage;
	_directiveMap["client_max_body_size"] = &ConfigParser::handleCliMaxSize;
	_directiveMap["autoindex"] = &ConfigParser::handleAutoIndex;
	// _directiveMap["redirect"] = &ConfigParser::handleRedirect;
}

static bool isValidPort(const std::string& port) {
	for (std::string::size_type i = 0; i < port.size(); ++i) {
		if (!std::isdigit(port[i])) return false;
	}
	int portNumber = std::atoi(port.c_str());
	return (portNumber >= 0 && portNumber <= 65535);
}

static bool isValidIp(const std::string& ip) {
	if (ip.empty()) return true;
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

// TODO: implement IPv6? default_server ?
// test what happens in NGINX address:<nothing> or <nothing>:port, 
// is it the same as not including that parameter?
void ConfigParser::handleListen(Context &context,
								std::vector<std::string> &tokens) {
	std::vector<std::string>::const_iterator it;
	for (it = tokens.begin() + 1; it != tokens.end(); it++){
		Listen listening;
		size_t colonPos = (*it).find(':');
		if (colonPos != std::string::npos) {
			listening.IP = (*it).substr(0, colonPos);
			listening.port = (*it).substr(colonPos + 1);
			if (listening.IP.empty() || listening.port.empty())
				throw ConfigError("Invalid IP:Port.");
		} else {
			if ((*it).find_first_not_of("0123456789") == std::string::npos)
				listening.port = (*it);
			else
				listening.IP = (*it);
		}
		if (listening.IP.empty())
			listening.IP = ""; // default
		if (listening.port.empty())
			listening.port = "80"; // default
		if (!isValidIp(listening.IP) || !isValidPort(listening.port))
			throw ConfigError("Invalid IP:Port.");
		context.network_address.push_back(listening);
	}
}

void ConfigParser::handleServerName(Context &context,
									std::vector<std::string> &tokens) {
	tokens.erase(tokens.begin());
	context.serverName = tokens;
}

void ConfigParser::handleRoot(Context &context,
							  std::vector<std::string> &tokens) {
	if (tokens.size() > 2)
		throw ConfigError("Invalid root directive.");
	context.root = tokens[1];
}

void ConfigParser::handleIndex(Context &context,
							   std::vector<std::string> &tokens) {
	tokens.erase(tokens.begin());
	context.index = tokens;
}

void ConfigParser::handleLimitExcept(Context &context,
									 std::vector<std::string> &tokens) {
	std::vector<std::string>::const_iterator it;
	for (it = tokens.begin() + 1; it != tokens.end(); it++){
		if ((*it) == "GET")
			context.allowedMethods.push_back(GET);
		else if ((*it) == "DELETE")
			context.allowedMethods.push_back(DELETE);
		else if ((*it) == "POST")
			context.allowedMethods.push_back(POST);
		else
			throw ConfigError("Unsupported method detected.");
	}
}

void ConfigParser::handleTryFiles(Context &context,
								  std::vector<std::string> &tokens) {
	tokens.erase(tokens.begin());
	context.tryFiles = tokens;
}

void ConfigParser::handleErrorPage(Context &context,
								   std::vector<std::string> &tokens) {
	if (tokens.size() < 3)
		throw ConfigError("Invalid error_page directive.");
	std::string page = tokens.back();
	for (size_t i = 1; i < tokens.size() - 1; i++){
		char *end;
		long statusCodeLong = std::strtol(tokens[i].c_str(), &end, 10);
		if (*end != '\0' || statusCodeLong < 100 || statusCodeLong > 599 \
		|| statusCodeLong != static_cast<short>(statusCodeLong))
				throw ConfigError("Invalid status code in error_page directive.");
		context.errorPages[static_cast<short>(statusCodeLong)] = page;
	}
}

void ConfigParser::handleCliMaxSize(Context &context,
									std::vector<std::string> &tokens) {
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
		context.clientMaxBodySize = size;
		break;
	case 'k':
	case 'K':
		if (size > maxLimit / 1024)
			throw ConfigError("client_max_body_size value overflow.");
		context.clientMaxBodySize = size * 1024;
		break;
	case 'm':
	case 'M':
		if (size > maxLimit / 1048576)
			throw ConfigError("client_max_body_size value overflow.");
		context.clientMaxBodySize = size * 1048576;
		break;
	case 'g':
	case 'G':
		if (size > maxLimit / 1073741824)
			throw ConfigError("client_max_body_size value overflow.");
		context.clientMaxBodySize = size * 1073741824;
		break;
	default:
		throw ConfigError("Invalid unit for client_max_body_size.");
	}
}

void ConfigParser::handleAutoIndex(Context &context,
								   std::vector<std::string> &tokens) {
	context.autoIndex = FALSE;
	if (tokens[1] == "on")
		context.autoIndex = TRUE;
	else if (tokens[1] != "off")
		throw ConfigError("Invalid syntax.");
}

size_t ConfigParser::advanceBlock(std::string content, size_t start) {
	short scope = 0;
	while (content[start]) {
		char c = content[start];
		if (c == '{')
			scope++;
		else if (c == '}') {
			scope--;
			if (scope == 0) return (start);
		}
		start++;
	}
	throw ConfigError("Unmatched '{}'.");
}

std::vector<std::string> ConfigParser::splitServerBlocks(std::string content) {
	size_t start = 0;
	size_t end = 0;
	size_t endBlockContent = 0;
	std::vector<std::string> servers;
	std::string firstWord;
	while (content[start]) {
		firstWord = (content.substr(start, 6));
		if (stringToLower(firstWord) != "server")
			throw ConfigError("No server block present.");
		start += 6;
		while (std::isspace(content[start])) start++;
		if (content[start] != '{')
			throw ConfigError("No '{' at beginning of block.");
		end = advanceBlock(content, start);
		while (std::isspace(content[++start])) {
		}
		endBlockContent = end;
		while (std::isspace(content[--endBlockContent])) {
		}
		servers.push_back(content.substr(start, endBlockContent - start + 1));
		start = end + 1;
		while (content[start] && std::isspace(content[start])) start++;
	}
	return (servers);
}

// Splits a string into a vector of strings, removing WS
std::vector<std::string> ConfigParser::tokenizeLine(std::string line) {
	std::vector<std::string> tokens;
	std::string value;
	std::stringstream strStream(line);

	while (strStream >> value) {
		trimOuterSpaces(value);
		tokens.push_back(value);
	}
	return (tokens);
}

void ConfigParser::processDirective(Context &server, std::string &line) {
	std::vector<std::string> tokens;
	tokens = tokenizeLine(line);
	if (tokens.size() < 2)
		throw ConfigError("No value for directive: " + tokens[0]);
	std::map<std::string, DirectiveHandler>::const_iterator it;
	it = _directiveMap.find(tokens[0]);
	if (it != _directiveMap.end())
		(this->*(it->second))(server, tokens);
	else
		throw ConfigError("Unkown directive: " + tokens[0]);
}

// TODO: improve flow of function
//		load inherited values?
void ConfigParser::processLocation(Context &server, std::string block,
								   size_t start, size_t end) {
	std::string route;
	std::string line;
	std::string firstWord;
	std::vector<std::string> tokens;
	std::istringstream location(block.substr(start, end - start));
	Context locationInfo;

	location >> route;  				// Discard 'location'
	location >> route;  				// Get the actual route
	if (route == "{")
		throw ConfigError("No location route.");
	location >> line;  // Discard the opening '{'
	if (line != "{")
		throw ConfigError("Location can only support one route.");
	while (std::getline(location, line, ';'))
	{
		trimOuterSpaces(line);
		if ((line.empty() && !location.eof()))
			throw ConfigError("Unparsable location block detected.");
		if (line.empty())
			continue;
		std::istringstream readLine(line);
		readLine >> firstWord;
		if (stringToLower(firstWord) == "location")
			throw ConfigError("Nested locations not supported.");
		if (stringToLower(firstWord) == "server_name" || stringToLower(firstWord) == "listen")
			throw ConfigError("Directive " + firstWord + " not supported in location context.");
		processDirective(locationInfo, line);
		if (location.tellg() >= static_cast<std::streampos>(end)) 
			break;
	}
	server.locations[route] = locationInfo;
}

void ConfigParser::loadIntoContext(std::vector<std::string> &blocks) {
	std::string line;
	std::vector<std::string>::iterator it;
	std::string firstWord;

	for (it = blocks.begin(); it != blocks.end(); it++) {
		Context server;
		loadDefaults(server);
		std::istringstream block(*it);
		std::streampos startPos = block.tellg();
		while (std::getline(block, line,
							';')) {
			trimOuterSpaces(line);
			if (line.empty()) throw ConfigError("Unparsable block detected.");
			std::istringstream readLine(line);
			readLine >> firstWord;
			if (stringToLower(firstWord) == "location") {
				size_t endPos = (*it).find("}", startPos);
				processLocation(server, (*it), startPos, endPos);
				std::getline(block, line, '}');
			} else if (stringToLower(firstWord) == "limit_except")
				throw ConfigError("limit_except not allowed in server context.");
			else if (stringToLower(firstWord) == "}")
				break ;
			else
				processDirective(server, line);
			startPos = block.tellg();
		}
		_servers.push_back(server);
	}
}

void ConfigParser::loadConfigs() {
	std::ifstream file(_configFile.c_str());
	if (!file.is_open())
		throw ConfigError("Unable to read from: " + _configFile);
	std::string fileContents;
	// Read file contents into the string
	fileContents.assign((std::istreambuf_iterator<char>(file)),
							(std::istreambuf_iterator<char>()));
	file.close();
	trimOuterSpaces(fileContents);
	trimComments(fileContents);
	if (fileContents.empty())
		throw ConfigError("Configuration file is empty.");
	std::vector<std::string> serverBlocks;
	serverBlocks = splitServerBlocks(fileContents);
	loadIntoContext(serverBlocks);
}

std::vector<Context> ConfigParser::getServers(void) { return (this->_servers); }

void ConfigParser::printContext(Context context) {
	std::vector<std::string>::const_iterator it2;
	if (!context.network_address.empty()) {
		std::cout << "Network Addresses: " << std::endl;
		std::vector<Listen>::const_iterator it;
		for (it = context.network_address.begin(); it != context.network_address.end(); ++it){
			std::cout << "IP: " << (*it).IP << " Port: " << (*it).port << '\n';
		}
	}
	if (!context.serverName.empty()) {
		std::cout << "Server Name: ";
		for (it2 = context.serverName.begin(); it2 != context.serverName.end();
			 ++it2)
			std::cout << *it2 << " ";
		std::cout << std::endl;
	}
	if (!context.root.empty())
		std::cout << "Root: " << context.root << std::endl;
	if (!context.index.empty()) {
		std::cout << "Index: ";
		for (it2 = context.index.begin(); it2 != context.index.end(); ++it2)
			std::cout << *it2 << " ";
		std::cout << std::endl;
	}
	std::cout << "Auto Index: " << (context.autoIndex ? "Enabled" : "Disabled")
			  << std::endl;
	std::cout << "Client Max Body Size: " << context.clientMaxBodySize
			  << std::endl;
	if (!context.uploadDir.empty())
		std::cout << "Upload Directory: " << context.uploadDir << std::endl;
	if (!context.tryFiles.empty()) {
		std::cout << "Try Files: ";
		for (it2 = context.tryFiles.begin(); it2 != context.tryFiles.end();
			 ++it2)
			std::cout << *it2 << " ";
		std::cout << std::endl;
	}
	if (!context.allowedMethods.empty()) {
		std::cout << "Allowed Methods: ";
		std::vector<Method>::const_iterator it3;
		for (it3 = context.allowedMethods.begin();
			 it3 != context.allowedMethods.end(); it3++) {
			switch (*it3) {
				case GET:
					std::cout << "GET ";
					break;
				case POST:
					std::cout << "POST ";
					break;
				case DELETE:
					std::cout << "DELETE ";
					break;
				default:
					std::cout << "UNKNOWN ";
			}
		}
		std::cout << std::endl;
	}
	if (!context.errorPages.empty()) {
		std::cout << "Error Pages: " << std::endl;
		std::map<short, std::string>::const_iterator it;
		for (it = context.errorPages.begin(); it != context.errorPages.end();
			 ++it)
			std::cout << "  Error " << it->first << ": " << it->second
					  << std::endl;
	}
	if (!context.locations.empty()) {
		std::cout << "Locations: " << std::endl;
		std::map<std::string, Context>::const_iterator it;
		for (it = context.locations.begin(); it != context.locations.end();
			 ++it) {
			std::cout << it->first << ": \n";
			printContext(it->second);
		}
	}
}

void ConfigParser::printConfigs() {
	if (_servers.begin() == _servers.end()) {
		std::cout << "No configs loaded.\n";
		return;
	}
	std::vector<Context>::const_iterator it;
	for (it = _servers.begin(); it != _servers.end(); it++) {
		printContext(*it);
		std::cout << std::endl;
	}
}
