/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Exceptions.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/13 12:30:13 by damachad          #+#    #+#             */
/*   Updated: 2024/08/14 11:37:06 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXCEPTIONS_HPP
# define EXCEPTIONS_HPP

# include "Webserv.hpp"

class FileReadError : public std::exception
{
	private:
		std::string _message;
	public:
		FileReadError(const std::string &str) throw();
		virtual const char *what() const throw();
		virtual ~FileReadError() throw();
};

class ConfigError : public std::exception
{
	private:
		std::string _message;
	public:
		ConfigError(const std::string &str) throw();
		virtual const char *what() const throw();
		virtual ~ConfigError() throw();
};

#endif
