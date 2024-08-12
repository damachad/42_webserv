/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/12 14:13:04 by damachad          #+#    #+#             */
/*   Updated: 2024/08/12 15:23:59 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include "defines.hpp"

enum Method
{
	GET = 1,
	POST,
	DELETE
};

typedef std::map<std::string, Context> Locations;

struct Context
{
	std::string						host;
	uint16_t						port;
	std::string						serverName;
	std::string						root;
	std::string						index;
	bool							autoIndex;
	unsigned long					clientMaxBodySize;
	std::string						uploadDir;
	std::string						tryFile;
	std::vector<Method>				allowedMethods;
	std::map<short, std::string>	errorPages;
	Locations						locations;
};

class ConfigParser
{
	private:
		std::vector<Context>		_servers;

	public:
		ConfigParser();
		ConfigParser(const std::string &file);
		~ConfigParser();

		void	loadConfigs(const std::string &file);
		void	loadDefaults();
};

#endif
