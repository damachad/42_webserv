/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Exceptions.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/13 12:30:13 by damachad          #+#    #+#             */
/*   Updated: 2024/09/13 20:06:47 by mde-sa--         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include "Helpers.hpp"
#include "Webserv.hpp"

class FileReadError : public std::exception {
   private:
	std::string _message;

   public:
	FileReadError(const std::string &str) throw();
	virtual const char *what() const throw();
	virtual ~FileReadError() throw();
};

class ConfigError : public std::exception {
   private:
	std::string _message;

   public:
	ConfigError(const std::string &str) throw();
	virtual const char *what() const throw();
	virtual ~ConfigError() throw();
};

class OutOfBoundsError : public std::exception {
   private:
	std::string _message;

   public:
	OutOfBoundsError(const std::string &str) throw();
	virtual const char *what() const throw();
	virtual ~OutOfBoundsError() throw();
};

class ValueNotFoundError : public std::exception {
   private:
	std::string _message;

   public:
	ValueNotFoundError(const std::string &str) throw();
	virtual const char *what() const throw();
	virtual ~ValueNotFoundError() throw();
};

class SocketSetupError : public std::exception {
   private:
	std::string _message;

   public:
	SocketSetupError(const std::string &str) throw();
	virtual const char *what() const throw();
	virtual ~SocketSetupError() throw();
};

class ClusterSetupError : public std::exception {
   private:
	std::string _message;

   public:
	ClusterSetupError(const std::string &str) throw();
	virtual const char *what() const throw();
	virtual ~ClusterSetupError() throw();
};

class ClusterRunError : public std::exception {
   private:
	std::string _message;

   public:
	ClusterRunError(const std::string &str) throw();
	virtual const char *what() const throw();
	virtual ~ClusterRunError() throw();
};

class HTTPHeaderError : public std::exception {
   private:
	std::string _message;

   public:
	HTTPHeaderError(const std::string &str) throw();
	virtual const char *what() const throw();
	virtual ~HTTPHeaderError() throw();
};

class HTTPResponseError : public std::exception {
   private:
	std::string _response;

   public:
	HTTPResponseError(const short status) throw();
	virtual const char *what() const throw();
	virtual ~HTTPResponseError() throw();
};

#endif
