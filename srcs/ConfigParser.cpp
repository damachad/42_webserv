/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/13 11:25:49 by damachad          #+#    #+#             */
/*   Updated: 2024/08/14 17:31:11 by damachad         ###   ########.fr       */
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
	server.index = "index.html";
	server.clientMaxBodySize = 1048576; // 1m
	// server.root = "";
	// server.serverName = "localhost";
}

/* remove leading and trailing whitespaces*/
void	ConfigParser::trimOuterSpaces(std::string &s)
{
	if (s.empty())
		return ;
	while (std::isspace(s[0]))
		s.erase(s.begin());
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

// TODOs: validate if block ID is just 'server' (e.g. 'serverr')
// 		validate consecutive '{' and '}' here? 
// 		(flexible function for location blocks?)
std::vector<std::string>	ConfigParser::splitServerBlocks(std::string content)
{
	size_t start = 0;
	size_t end = 0;
	size_t endBlockContent = 0;
	std::vector<std::string> servers;
	while (content[start])
	{
		if (content.compare(start, 6, "server"))
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

// TODO: get tokens from line
std::vector<std::string>	ConfigParser::processLine(std::string line)
{
	static int i = 1;
	std::vector<std::string> tokens;
	trimOuterSpaces(line);
	std::cout << "Line " << i << ": " << line << "\n";
	i++;
	return (tokens);
}

// TODO: fix segfault
void	ConfigParser::loadIntoContext(std::vector<std::string> &blocks)
{
	std::vector<std::string> tokens;
	std::string line;
	std::vector<std::string>::iterator it;
	for (it = blocks.begin(); it != blocks.end(); it++){
		std::istringstream block(*it);
		while (std::getline(block, line)){
			// turn each line of server block into vector of tokens
			tokens = processLine(line);
			// specific function to parse location blocks
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
		std::cout << "SERVER BLOCKS\n";
		std::vector<std::string>::iterator it;
		for (it = serverBlocks.begin(); it != serverBlocks.end(); it++){
			std::cout << *it << '\n';
			std::cout << "----------------------------\n";
		}
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
	if (!context.ports.empty())
	{
		std::cout << "Ports: " << std::endl;
		std::vector<u_int16_t>::iterator it;
		for (it = context.ports.begin(); it != context.ports.end(); ++it)
			std::cout << *it << " ";
		std::cout << std::endl;
	}
	if (!context.serverName.empty())
		std::cout << "Server Name: " << context.serverName << std::endl;
	if (!context.root.empty())
		std::cout << "Root: " << context.root << std::endl;
	if (!context.index.empty())
		std::cout << "Index: " << context.index << std::endl;
	std::cout << "Auto Index: " << (context.autoIndex ? "Enabled" : "Disabled") << std::endl;
	std::cout << "Client Max Body Size: " << context.clientMaxBodySize << std::endl;
	if (!context.uploadDir.empty())
		std::cout << "Upload Directory: " << context.uploadDir << std::endl;
	if (!context.tryFile.empty())
		std::cout << "Try File: " << context.tryFile << std::endl;
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
		Locations::iterator it;
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
	std::vector<Context>::iterator it;
	for (it = _servers.begin(); it != _servers.end(); it++)
		printContext(*it);
}
