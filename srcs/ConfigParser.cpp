/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/13 11:25:49 by damachad          #+#    #+#             */
/*   Updated: 2024/08/19 16:12:50 by damachad         ###   ########.fr       */
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

// Necessary?
void ConfigParser::loadDefaults() {
	Context server;
	// default values from NGINX
	// server.networkaddress.push_back(80);
	// server.index.push_back("index.html");
	server.clientMaxBodySize = 1048576;	 // 1m
										 //  server.root = "";
										 //  server.serverName = "localhost";
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

// TODO: Divide into IP and Port, use default IP if not provided
void ConfigParser::handleListen(Context &context,
								std::vector<std::string> &tokens) {
	(void)context;
	(void)tokens;
	// std::vector<std::string>::const_iterator it;
	// std::cout << "Handling: ";
	// for (it = tokens.begin(); it != tokens.end(); it++){
	// 	std::cout << (*it) << " ";
	// }
	// std::cout << std::endl;
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

// TODO: parse errors unto the map (str to short) and pair with last arg (file)
void ConfigParser::handleErrorPage(Context &context,
								   std::vector<std::string> &tokens) {
	(void)context;
	(void)tokens;
	// std::vector<std::string>::const_iterator it;
	// std::cout << "Handling: ";
	// for (it = tokens.begin(); it != tokens.end(); it++){
	// 	std::cout << (*it) << " ";
	// }
	// std::cout << std::endl;
}

// TODO: parse string to unsigned long based on the unit
void ConfigParser::handleCliMaxSize(Context &context,
									std::vector<std::string> &tokens) {
	(void)context;
	(void)tokens;
	// if (tokens.empty())
	// 	std::cout << "No tokens passed to handler function\n";
	// std::vector<std::string>::const_iterator it;
	// std::cout << "Handling: ";
	// for (it = tokens.begin(); it != tokens.end(); it++){
	// 	std::cout << (*it) << " ";
	// }
	// std::cout << std::endl;
}

void ConfigParser::handleAutoIndex(Context &context,
								   std::vector<std::string> &tokens) {
	if (tokens[1] == "on")
		context.autoIndex = true;
	else if (tokens[1] == "off")
		context.autoIndex = false;
	else
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

// TODOs: validate consecutive '{' and '}' here?
// 		(flexible function for location blocks?)
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
// 		parse each directive unto Context struct
void ConfigParser::processLocation(Context &server, std::string block,
								   size_t start, size_t end) {
	std::string route;
	std::string line;
	std::string firstWord;
	std::vector<std::string> tokens;
	std::istringstream location(block.substr(start, end - start));
	Context locationInfo;
	bool 		empty = true;

	location >> route;  				// Discard 'location'
	location >> route;  				// Get the actual route
	if (route == "{")
		throw ConfigError("No location route.");
	location >> line;  // Discard the opening '{'
	if (line != "{")
		throw ConfigError("Location can only support one route.");
	while (std::getline(location, line, ';')) // change this, there may be no ';'
	{
		trimOuterSpaces(line);
		if ((location.eof() && empty) || (line.empty() && !location.eof()))
			throw ConfigError("Unparsable location block detected.");
		if (line.empty())
			continue;
		firstWord = line.substr(0, 8);
		if (stringToLower(firstWord) == "location")
			throw ConfigError("Nested locations not supported.");
		else {
			processDirective(locationInfo, line);
			empty = false;
		}
		if (location.tellg() >= static_cast<std::streampos>(end)) 
			break;
	}
	server.locations[route] = locationInfo;
}

// TODO: Fix content loading (clear struct ?)
void ConfigParser::loadIntoContext(std::vector<std::string> &blocks) {
	std::string line;
	std::vector<std::string>::iterator it;
	std::string firstWord;
	Context server;

	for (it = blocks.begin(); it != blocks.end(); it++) {
		std::istringstream block(*it);
		std::streampos startPos = block.tellg();
		while (std::getline(block, line,
							';')) {	 // change this, there may be no ';'
			trimOuterSpaces(line);
			if (line.empty()) throw ConfigError("Unparsable block detected.");
			firstWord = line.substr(0, 8);
			if (stringToLower(firstWord) == "location") {
				size_t endPos = (*it).find("}", startPos);
				processLocation(server, (*it), startPos, endPos);
				std::getline(block, line, '}');
			} else {
				processDirective(server, line);
			}
			startPos = block.tellg();
		}
		_servers.push_back(server);
	}
}

// TODO: improve error handling
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
			std::cout << it->first << ": ";
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
