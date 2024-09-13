/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/12 14:13:04 by damachad          #+#    #+#             */
/*   Updated: 2024/08/21 16:48:41 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

class Server;

#include "Server.hpp"
#include "Webserv.hpp"

class ConfigParser {
   private:
	std::string _configFile;
	std::vector<Server> _servers;

   public:
	ConfigParser();
	ConfigParser(const ConfigParser &src);
	ConfigParser(const std::string &file);
	~ConfigParser();
	ConfigParser &operator=(const ConfigParser &src);

	void loadConfigs();
	void trimComments(std::string &s);
	void loadIntoContext(std::vector<std::string> &blocks);
	size_t advanceBlock(std::string content, size_t start);
	std::vector<std::string> splitServerBlocks(std::string content);
	void processDirective(Server &server, std::string &line);
	std::vector<Server> getServers(void);

	void printLocationValues(unsigned int serverNumconst,
							 const std::string &route);
	static std::vector<std::string> tokenizeLine(std::string line);
	static void trimOuterSpaces(std::string &s);
};

// Utils
std::string stringToLower(std::string str);
std::string stringToUpper(std::string str);

#endif
