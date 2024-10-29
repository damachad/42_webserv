/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/12 14:13:04 by damachad          #+#    #+#             */
/*   Updated: 2024/10/17 16:18:41 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

class Server;

#include "Server.hpp"
#include "Webserv.hpp"

class ConfigParser {
   private:
	std::string _config_file;
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
	std::vector<Server> getServers(void);

	void printLocationValues(unsigned int serverNumconst,
							 const std::string &route);
	static std::vector<std::string> tokenizeLine(std::string line);
	static void trimOuterSpaces(std::string &s);
};

#endif
