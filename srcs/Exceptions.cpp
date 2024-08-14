/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Exceptions.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/13 12:31:46 by damachad          #+#    #+#             */
/*   Updated: 2024/08/14 12:35:47 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

FileReadError::FileReadError(const std::string &str) throw()
{
	_message = "Unable to read from: " + str + '\n';
}

const char *FileReadError::what() const throw()
{
	return (_message.c_str());
}

FileReadError::~FileReadError() throw(){}

/* Configuration File Errors */

ConfigError::ConfigError(const std::string &str) throw()
{
	_message = "Error in configuration file: " + str + '\n';
}

const char *ConfigError::what() const throw()
{
	return (_message.c_str());
}

ConfigError::~ConfigError() throw(){}
