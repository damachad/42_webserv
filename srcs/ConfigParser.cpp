/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/13 11:25:49 by damachad          #+#    #+#             */
/*   Updated: 2024/08/16 18:24:05 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

ConfigParser::ConfigParser(void){}

// TODO: Implement
ConfigParser::ConfigParser(const ConfigParser &src)
{
	*this = src;
}

ConfigParser::ConfigParser(const std::string &file) : _configFile(file)
{}

ConfigParser::~ConfigParser(void)
{}

ConfigParser &	ConfigParser::operator=(const ConfigParser & src)
{
	if (this != &src){
		this->_configFile = src._configFile;
		this->_servers = src._servers;
	}
	return (*this);
}

void	ConfigParser::loadDefaults()
{
	Context	server;
	// default values from NGINX
	server.ports.push_back(80);
	server.index.push_back("index.html");
	server.clientMaxBodySize = 1048576; // 1m
	// server.root = "";
	// server.serverName = "localhost";
}

/* remove leading and trailing whitespaces */
void	ConfigParser::trimOuterSpaces(std::string &s)
{
	if (s.empty())
		return ;
	while (std::isspace(s[0]))
		s.erase(s.begin());
	if (s.empty()) // case where line only has WS
		return ;
	while (std::isspace(s[s.length() - 1]))
		s.erase(s.end() - 1);
}


/* remove comments */
void	ConfigParser::trimComments(std::string &s)
{
	if (s.empty())
		return ;
	size_t comment = s.find('#');
	while (comment != std::string::npos)
	{
		size_t endl = s.find('\n', comment);
		s.erase(comment, endl - comment);
		comment = s.find('#');
	}
}

size_t	ConfigParser::advanceBlock(std::string content, size_t start)
{
	short scope = 0;
	while (content[start])
	{
		char c = content[start];
		if (c == '{')
			scope++;
		else if (c == '}')
		{
			scope--;
			if (scope == 0)
				return (start);
		}
		start++;
	}
	throw ConfigError("Unmatched '{}'.");
}

// TODOs: validate consecutive '{' and '}' here? 
// 		(flexible function for location blocks?)
std::vector<std::string>	ConfigParser::splitServerBlocks(std::string content)
{
	size_t start = 0;
	size_t end = 0;
	size_t endBlockContent = 0;
	std::vector<std::string> servers;
	std::string firstWord;
	while (content[start])
	{
		firstWord = (content.substr(start, 6));
		if (stringToLower(firstWord) != "server")
			throw ConfigError("No server block present.");
		start += 6;
		while (std::isspace(content[start]))
			start++;
		if (content[start] != '{')
			throw ConfigError("No '{' at beginning of block.");
		end = advanceBlock(content, start);
		while (std::isspace(content[++start])){}
		endBlockContent = end;
		while (std::isspace(content[--endBlockContent])){}
		servers.push_back(content.substr(start, endBlockContent - start + 1));
		start = end + 1;
		while (content[start] && std::isspace(content[start]))
			start++;
	}
	return (servers);
}

// Splits a string into a vector of strings, removing WS
std::vector<std::string>	ConfigParser::tokenizeLine(std::string line)
{
	std::vector<std::string>	tokens;
	std::string					value;
	std::stringstream strStream(line);

	while (strStream >> value){
		trimOuterSpaces(value);
		tokens.push_back(value);
	}
	return (tokens);
}

// TODO: parse directive unto Context struct
void	ConfigParser::processDirective(Context &server, std::string line)
{
	(void)server;
	std::vector<std::string> tokens;
	tokens = tokenizeLine(line);
	
}

// TODO: improve flow of function
// 		parse each directive unto Context struct
void	ConfigParser::processLocation(Context &server, std::string block, size_t start, size_t end)
{
	(void)server;
	std::string route;
	std::string line;
	std::vector<std::string> tokens;
	std::istringstream location(block.substr(start, end - start));
	location >> route; // discard 'location'
	location >> route;
	location >> line; // apply a better approach ? Just discarding the '{'
	// std::cout << "LOCATION BLOCK\n";
	// std::cout << "Route: " << route << '\n';
	while (std::getline(location, line, ';')){
		trimOuterSpaces(line);
		if (line.empty())
			continue ;
		tokens = tokenizeLine(line);
		// std::vector<std::string>::const_iterator it2;
		// for (it2 = tokens.begin(); it2 != tokens.end(); it2++)
		// 	std::cout << (*it2) << " ";
		// std::cout << std::endl;
	}
}

void	ConfigParser::loadIntoContext(std::vector<std::string> &blocks)
{
	std::string line;
	std::vector<std::string>::iterator it;
	std::string firstWord;
	Context	server;
	for (it = blocks.begin(); it != blocks.end(); it++){
		std::istringstream block(*it);
		std::streampos startPos = block.tellg();
		while (std::getline(block, line, ';')){
			trimOuterSpaces(line);
			if (line.empty())
				continue ;
			firstWord = line.substr(0, 8);
			if (stringToLower(firstWord) == "location"){
				size_t endPos = (*it).find("}", startPos);
				processLocation(server, (*it), startPos, endPos);
				std::getline(block, line, '}');
			}
			else {
				processDirective(server, line);

			}
			startPos = block.tellg();
			// check if directive is one covered by this program
		}
	}
}

// TODO: improve error handling
bool	ConfigParser::loadConfigs()
{	
	std::ifstream file(_configFile.c_str());
	if (!file.is_open())
	{
		std::cerr << "Unable to read from: " << _configFile << "\n";
		// call error function that exits
		return (false);
	}
	else
	{
		std::string fileContents;
		// Read file contents into the string
		try{
			fileContents.assign((std::istreambuf_iterator<char>(file)),
						(std::istreambuf_iterator<char>()));
		}
		catch (std::exception &e){
			std::cerr << e.what() << std::endl;
			// call error function that exits
			return (false);
		}
		file.close();
		trimOuterSpaces(fileContents);
		trimComments(fileContents);
		if (fileContents.empty())
		{
			std::cerr << "Configuration file is empty.\n";
			// call error function that exits
			return (false);
		}
		// std::cout << "CONFIG FILE\n" << fileContents << '\n';
		std::vector<std::string> serverBlocks;
		try{
			serverBlocks = splitServerBlocks(fileContents);
		}
		catch (std::exception &e){
			std::cerr << e.what();
			return (false);
		}
		// std::cout << "SERVER BLOCKS\n";
		// std::vector<std::string>::const_iterator it;
		// for (it = serverBlocks.begin(); it != serverBlocks.end(); it++){
		// 	std::cout << *it << '\n';
		// 	std::cout << "----------------------------\n";
		// }
		loadIntoContext(serverBlocks);
	}
	return (true);
}

std::vector<Context>	ConfigParser::getServers(void)
{
	return (this->_servers);
}

void	ConfigParser::printContext(Context context)
{
	std::vector<std::string>::const_iterator it2;
	if (!context.ports.empty())
	{
		std::cout << "Ports: " << std::endl;
		std::vector<u_int16_t>::const_iterator it;
		for (it = context.ports.begin(); it != context.ports.end(); ++it)
			std::cout << *it << " ";
		std::cout << std::endl;
	}
	if (!context.serverName.empty()){
		std::cout << "Server Name: " << std::endl;
		for (it2 = context.serverName.begin(); it2 != context.serverName.end(); ++it2)
			std::cout << *it2 << " ";
		std::cout << std::endl;
	}
	if (!context.root.empty())
		std::cout << "Root: " << context.root << std::endl;
	if (!context.index.empty()){
		std::cout << "Index: " << std::endl;
		for (it2 = context.index.begin(); it2 != context.index.end(); ++it2)
			std::cout << *it2 << " ";
		std::cout << std::endl;
	}
	std::cout << "Auto Index: " << (context.autoIndex ? "Enabled" : "Disabled") << std::endl;
	std::cout << "Client Max Body Size: " << context.clientMaxBodySize << std::endl;
	if (!context.uploadDir.empty())
		std::cout << "Upload Directory: " << context.uploadDir << std::endl;
	if (!context.tryFiles.empty()){
		std::cout << "Try Files: " << std::endl;
		for (it2 = context.tryFiles.begin(); it2 != context.tryFiles.end(); ++it2)
			std::cout << *it2 << " ";
		std::cout << std::endl;
	}
	if (!context.allowedMethods.empty())
	{
		std::cout << "Allowed Methods: ";
		std::vector<Method>::const_iterator it;
		for (it = context.allowedMethods.begin(); it != context.allowedMethods.end(); it++) {
			switch (*it) {
				case GET: std::cout << "GET "; break;
				case POST: std::cout << "POST "; break;
				case DELETE: std::cout << "DELETE "; break;
				default: std::cout << "UNKNOWN ";
			}
		}
		std::cout << std::endl;
	}
	if (!context.errorPages.empty())
	{
		std::cout << "Error Pages: " << std::endl;
		std::map<short, std::string>::const_iterator it;
		for (it = context.errorPages.begin(); it != context.errorPages.end(); ++it)
			std::cout << "  Error " << it->first << ": " << it->second << std::endl;
	}
	if (!context.locations.empty())
	{
		std::cout << "Locations: " << std::endl;
		std::map<std::string, Context>::const_iterator it;
		for (it = context.locations.begin(); it != context.locations.end(); ++it) {
			std::cout << it->first << ": ";
			printContext(it->second);
			std::cout << std::endl;
		}
	}
}

void	ConfigParser::printConfigs()
{
	if (_servers.begin() == _servers.end())
	{
		std::cout << "No configs loaded.\n";
		return ;
	}
	std::vector<Context>::const_iterator it;
	for (it = _servers.begin(); it != _servers.end(); it++)
		printContext(*it);
}
