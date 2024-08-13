/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/13 11:25:49 by damachad          #+#    #+#             */
/*   Updated: 2024/08/13 16:36:49 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

ConfigParser::ConfigParser(void){}

ConfigParser::ConfigParser(ConfigParser const &src)
{
	*this = src;
}

ConfigParser::ConfigParser(const std::string &file) : _configFile(file)
{}

ConfigParser::~ConfigParser(void)
{}

ConfigParser &	ConfigParser::operator=(ConfigParser const & src)
{
	if (this != &src)
		this->_servers = src._servers;
	return (*this);
}

void	ConfigParser::loadDefaults()
{
	Context	server;
	// default values from NGINX
	server.port = 80;
	server.index = "index.html";
	server.clientMaxBodySize = 1048576; // 1m
	// server.root = "";
	// server.host = "localhost";
	// server.serverName = "";
}

bool	ConfigParser::loadConfigs()
{
	// is access() necessary if you have file.is_open() ?
	
	// if (access(_configFile.c_str(), R_OK))
	// {
	// 	std::cerr << "Unable to read from: " << _configFile << "\n";
	// 	// call error function that exits
	// 	return (false);
	// }
	// loadDefaults();
	std::ifstream file;
	file.open(_configFile);
	if (!file.is_open())
	{
		std::cerr << "Unable to read from: " << _configFile << "\n";
		// call error function that exits
		return (false);
	}
	return (true);
}

std::vector<Context>	ConfigParser::getServers(void)
{
	return (this->_servers);
}

void	ConfigParser::printContext(Context context)
{
	if (!context.host.empty())
		std::cout << "Host: " << context.host << std::endl;
	std::cout << "Port: " << context.port << std::endl;
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
		for (it = context.locations.begin(); it != context.locations.end(); ++it)
			std::cout << it->first << ": ";
			printContext(it->second);
			std::cout << std::endl;
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
