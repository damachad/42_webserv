/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/13 11:25:49 by damachad          #+#    #+#             */
/*   Updated: 2024/08/13 11:40:46 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

ConfigParser::ConfigParser(void){}

ConfigParser::ConfigParser(ConfigParser const &src)
{
	*this = src;
}

ConfigParser::ConfigParser(const std::string &file)
{

}

ConfigParser::~ConfigParser(void)
{}

ConfigParser &	ConfigParser::operator=(ConfigParser const & src)
{
	if (this != &src)
		this->_servers = src._servers;
	return (*this);
}

std::vector<Context>	ConfigParser::getServers(void)
{
	return (this->_servers);
}
