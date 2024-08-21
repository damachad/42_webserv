/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerContext.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 11:47:36 by damachad          #+#    #+#             */
/*   Updated: 2024/08/21 12:28:14 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

ServerContext::ServerContext() : _autoIndex(FALSE), _clientMaxBodySize(1048576) {
	// NGINX defaults
	_index.push_back("index.html");
	_index.push_back("index.htm");
}

ServerContext::ServerContext(const ServerContext & src) : \
_network_address(src.getNetworkAddress()), \
_serverName(src.getServerName()), \
_root(src.getRoot()), \
_index(src.getIndex()), \
_autoIndex(src.getAutoIndex()), \
_clientMaxBodySize(src.getClientMaxBodySize()), \
_tryFiles(src.getTryFiles()), \
_errorPages(src.getErrorPages()), \
_locations(src.getLocations()) {}

ServerContext & ServerContext::operator=(const ServerContext & src) {
	_network_address = src.getNetworkAddress();
	_serverName = src.getServerName();
	_root = src.getRoot();
	_index = src.getIndex();
	_autoIndex = src.getAutoIndex();
	_clientMaxBodySize = src.getClientMaxBodySize();
	_tryFiles = src.getTryFiles();
	_errorPages = src.getErrorPages();
	_locations = src.getLocations();
}

ServerContext::~ServerContext() {}

// Getters
std::vector<Listen> ServerContext::getNetworkAddress() const {
	return _network_address;
}

std::vector<std::string> ServerContext::getServerName() const {
	return _serverName;
}

std::string ServerContext::getRoot() const {
	return _root;
}

std::vector<std::string> ServerContext::getIndex() const {
	return _index;
}

State ServerContext::getAutoIndex() const {
	return _autoIndex;
}

long ServerContext::getClientMaxBodySize() const {
	return _clientMaxBodySize;
}

std::vector<std::string> ServerContext::getTryFiles() const {
	return _tryFiles;
}

std::map<short, std::string> ServerContext::getErrorPages() const {
	return _errorPages;
}

std::map<std::string, LocationContext> ServerContext::getLocations() const {
	return _locations;
}

// Setters
void ServerContext::setNetworkAddress(const std::vector<Listen>& network_address) {
	this->_network_address = network_address;
}

void ServerContext::setServerName(const std::vector<std::string>& serverName) {
	this->_serverName = serverName;
}

void ServerContext::setRoot(const std::string& root) {
	this->_root = root;
}

void ServerContext::setIndex(const std::vector<std::string>& index) {
	this->_index = index;
}

void ServerContext::setAutoIndex(State autoIndex) {
	this->_autoIndex = autoIndex;
}

void ServerContext::setClientMaxBodySize(long clientMaxBodySize) {
	this->_clientMaxBodySize = clientMaxBodySize;
}

void ServerContext::setTryFiles(const std::vector<std::string>& tryFiles) {
	this->_tryFiles = tryFiles;
}

void ServerContext::setErrorPages(const std::map<short, std::string>& errorPages) {
	this->_errorPages = errorPages;
}

void ServerContext::setLocations(const std::map<std::string, LocationContext>& locations) {
	this->_locations = locations;
}

// Methods to add elements
void ServerContext::addNetworkAddress(const Listen& address) {
	_network_address.push_back(address);
}

void ServerContext::addServerName(const std::string& name) {
	_serverName.push_back(name);
}

void ServerContext::addIndex(const std::string& index) {
	this->_index.push_back(index);
}

void ServerContext::addTryFile(const std::string& tryFile) {
	_tryFiles.push_back(tryFile);
}

void ServerContext::addErrorPage(short code, const std::string& page) {
	_errorPages[code] = page;
}

void ServerContext::addLocation(const std::string& path, const LocationContext& context) {
	_locations[path] = context;
}
