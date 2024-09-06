/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Exceptions.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/13 12:31:46 by damachad          #+#    #+#             */
/*   Updated: 2024/09/06 15:27:23 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

FileReadError::FileReadError(const std::string &str) throw() {
	_message = "Unable to read from: " + str + '\n';
}

const char *FileReadError::what() const throw() { return (_message.c_str()); }

FileReadError::~FileReadError() throw() {}

/* Configuration File Errors */
ConfigError::ConfigError(const std::string &str) throw() {
	_message = "Error in configuration file: " + str + '\n';
}

const char *ConfigError::what() const throw() { return (_message.c_str()); }

ConfigError::~ConfigError() throw() {}

/* OutOfBounds Error for when accessing invalid index in vector */
OutOfBoundsError::OutOfBoundsError(const std::string &str) throw() {
	_message = "Error: out of bounds on index " + str + "!\n";
}

const char *OutOfBoundsError::what() const throw() {
	return (_message.c_str());
}

OutOfBoundsError::~OutOfBoundsError() throw() {}

/* ValueNotFound Error for when value is not found in vector */
ValueNotFoundError::ValueNotFoundError(const std::string &str) throw() {
	_message = "Error: could not find value " + str + "!\n";
}

const char *ValueNotFoundError::what() const throw() {
	return (_message.c_str());
}

ValueNotFoundError::~ValueNotFoundError() throw() {}

/* SocketSetup Error for when there are errors with socket / bind / listen */
SocketSetupError::SocketSetupError(const std::string &str) throw() {
	_message = "Error: could not run socket setup function " + str +
			   " (erro: " + strerror(errno) + ")\n";
}

const char *SocketSetupError::what() const throw() {
	return (_message.c_str());
}

SocketSetupError::~SocketSetupError() throw() {}

/* Cluster Setup Error for when there are errors with its setup */
ClusterSetupError::ClusterSetupError(const std::string &str) throw() {
	_message = "Error: could not execute cluster_setup function " + str +
			   " (errno: " + strerror(errno) + ")\n";
}

const char *ClusterSetupError::what() const throw() {
	return (_message.c_str());
}

ClusterSetupError::~ClusterSetupError() throw() {}

/* Cluster Run Error for when there are errors during its runtime */
ClusterRunError::ClusterRunError(const std::string &str) throw() {
	_message = "Error: could not run cluster_run function " + str +
			   " (errno: " + strerror(errno) + ")\n";
}

const char *ClusterRunError::what() const throw() { return (_message.c_str()); }

ClusterRunError::~ClusterRunError() throw() {}

/* HTTP Header error for when there are errors during its parsing */
HTTPHeaderError::HTTPHeaderError(const std::string &str) throw() {
	_message = "Wrong HTTP Header (" + str + ")\n";
}

const char *HTTPHeaderError::what() const throw() { return (_message.c_str()); }

HTTPHeaderError::~HTTPHeaderError() throw() {}

/* HTTP Response error for when there are errors during HTTP request interpretation */
HTTPResponseError::HTTPResponseError(const short status, 
	const std::multimap<std::string, std::string> & headers, 
	const std::string & body) throw() {
	
	std::map<short, std::string>::const_iterator itStatus = STATUS_MESSAGES.find(status);
	std::string message = (itStatus != STATUS_MESSAGES.end()) ? itStatus->second : "Unknown status code";
	std::ostringstream statusStream;
    statusStream << status;
	std::string headersStr;
	std::multimap<std::string, std::string>::const_iterator itHead;
	for (itHead = headers.begin(); itHead != headers.end(); itHead++) {
		headersStr += itHead->first + ": " + itHead->second + "\r\n";
	}
	_response = "HTTP1.1 " + statusStream.str() + message + "\r\n" + headersStr+ "\r\n\r\n" + body;
}

const char *HTTPResponseError::what() const throw() { return (_response.c_str()); }

HTTPResponseError::~HTTPResponseError() throw() {}
