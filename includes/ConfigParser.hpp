/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/12 14:13:04 by damachad          #+#    #+#             */
/*   Updated: 2024/08/16 11:30:07 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include "Webserv.hpp"

enum Method
{
	GET = 1,
	POST,
	DELETE
};

struct Context
{
	std::vector<uint16_t>			ports; // implement IP/port instead of just port ?
	std::string						serverName;
	std::string						root;
	std::string						index;
	bool							autoIndex;
	unsigned long					clientMaxBodySize;
	std::string						uploadDir;
	std::string						tryFile;
	std::vector<Method>				allowedMethods;
	std::map<short, std::string>	errorPages; 
	std::map<std::string, Context>	locations;
	// Later add redirect and cgi related variables
};

class ConfigParser
{
	private:
		std::string					_configFile;
		std::vector<Context>		_servers;

	public:
		ConfigParser();
		ConfigParser(const ConfigParser &src);
		ConfigParser(const std::string &file);
		~ConfigParser();
		ConfigParser & operator=(const ConfigParser &src);

		bool	loadConfigs();
		void	loadDefaults();
		void	printConfigs();
		void	printContext(Context context);
		void	trimOuterSpaces(std::string &s);
		void	trimComments(std::string &s);
		void	loadIntoContext(std::vector<std::string> &blocks);
		size_t		advanceBlock(std::string content, size_t start);
		std::vector<std::string>	splitServerBlocks(std::string content);
		std::vector<std::string>	processLine(std::string line);
		std::vector<Context>	getServers(void);
};

#endif
