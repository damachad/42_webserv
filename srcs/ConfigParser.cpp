/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/13 11:25:49 by damachad          #+#    #+#             */
/*   Updated: 2024/09/09 17:27:35 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"

#include "Exceptions.hpp"

ConfigParser::ConfigParser(void) {}

ConfigParser::ConfigParser(const ConfigParser &src) { *this = src; }

ConfigParser::ConfigParser(const std::string &file) : _configFile(file) {}

ConfigParser::~ConfigParser(void) {}

ConfigParser &ConfigParser::operator=(const ConfigParser &src) {
	if (this != &src) {
		this->_configFile = src._configFile;
		this->_servers = src._servers;
	}
	return (*this);
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
		ConfigParser::trimOuterSpaces(value);
		tokens.push_back(value);
	}
	return (tokens);
}

static bool areListenEqual(const Listen &a, const Listen &b) {
	return (a.port == b.port) && (a.IP == b.IP);
}

static bool hasDuplicates(const std::vector<Listen> &vec) {
	for (size_t i = 0; i < vec.size(); ++i) {
		for (size_t j = i + 1; j < vec.size(); ++j) {
			if (areListenEqual(vec[i], vec[j])) {
				return true;  // Duplicate found
			}
		}
	}
	return false;  // No duplicates
}

void ConfigParser::loadIntoContext(std::vector<std::string> &blocks) {
	std::string line;
	std::vector<std::string>::iterator it;
	std::string firstWord;

	for (it = blocks.begin(); it != blocks.end(); it++) {
		ServerContext server;
		std::istringstream block(*it);
		std::streampos startPos = block.tellg();
		while (std::getline(block, line, ';')) {
			trimOuterSpaces(line);
			if (line.empty()) throw ConfigError("Unparsable block detected.");
			std::istringstream readLine(line);
			readLine >> firstWord;
			if (stringToLower(firstWord) == "location") {
				size_t endPos = (*it).find("}", startPos);
				server.processLocation((*it), startPos, endPos);
				std::getline(block, line, '}');
			} else if (stringToLower(firstWord) == "}")
				break;
			else
				server.processDirective(line);
			startPos = block.tellg();
		}
		if (hasDuplicates(server.getNetworkAddress()))
			throw ConfigError("Duplicate network addresses found.");
		if (server.getRoot()
				.empty())  // TODO: enforce root directive of set default?
			throw ConfigError("No root directive present in server.");
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
	if (fileContents.empty()) throw ConfigError("Configuration file is empty.");
	std::vector<std::string> serverBlocks;
	serverBlocks = splitServerBlocks(fileContents);
	loadIntoContext(serverBlocks);
}

std::vector<ServerContext> ConfigParser::getServers(void) {
	return (this->_servers);
}

void ConfigParser::printLocationValues(unsigned int serverNum,
									   const std::string &route) {
	std::cout << "Getting diretive values from location '" << route;
	std::cout << "' in server[" << serverNum << "]:\n";
	std::cout << "(If not present in location, tries to get from server)\n\n";
	std::cout << "Root: " << _servers[serverNum].getRoot(route) << '\n';
	std::cout << "Index:\n";
	std::vector<std::string> indexFiles = _servers[serverNum].getIndex(route);
	for (std::vector<std::string>::const_iterator it = indexFiles.begin();
		 it != indexFiles.end(); ++it)
		std::cout << "  " << *it << "\n";
	std::cout << "AutoIndex: "
			  << (_servers[serverNum].getAutoIndex(route) == TRUE	 ? "TRUE"
				  : _servers[serverNum].getAutoIndex(route) == FALSE ? "FALSE"
																	 : "UNSET")
			  << "\n";
	std::cout << "Client Max Body Size: "
			  << _servers[serverNum].getClientMaxBodySize(route) << '\n';
	std::cout << "Try Files:\n";
	std::vector<std::string> tryFiles = _servers[serverNum].getTryFiles(route);
	for (std::vector<std::string>::const_iterator it = tryFiles.begin();
		 it != tryFiles.end(); ++it)
		std::cout << "  " << *it << "\n";
	std::cout << "Error Pages:\n";
	std::map<short, std::string> errorPages =
		_servers[serverNum].getErrorPages(route);
	for (std::map<short, std::string>::const_iterator it = errorPages.begin();
		 it != errorPages.end(); ++it)
		std::cout << "  " << it->first << " : " << it->second << "\n";
	std::cout << "Allowed Methods: ";
	std::set<Method> methods = _servers[serverNum].getAllowedMethods(route);
	for (std::set<Method>::const_iterator it = methods.begin();
		 it != methods.end(); ++it) {
		switch (*it) {
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
	std::cout << "Return: ";
	std::pair<short, std::string> returns =
		_servers[serverNum].getReturn(route);
	if (returns.first) std::cout << returns.first << " : " << returns.second;
	std::cout << std::endl;
}
