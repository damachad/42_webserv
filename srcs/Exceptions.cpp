/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Exceptions.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/13 12:31:46 by damachad          #+#    #+#             */
/*   Updated: 2024/10/17 16:32:47 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Exceptions.hpp"

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
