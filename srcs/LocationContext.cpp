/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationContext.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 11:47:36 by damachad          #+#    #+#             */
/*   Updated: 2024/08/21 12:28:14 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

LocationContext::LocationContext() : _autoIndex(FALSE), _clientMaxBodySize(1048576) {
	// NGINX defaults
	_index.push_back("index.html");
	_index.push_back("index.htm");
}

LocationContext::LocationContext(const LocationContext & src) : \
_root(src.getRoot()), \
_index(src.getIndex()), \
_autoIndex(src.getAutoIndex()), \
_clientMaxBodySize(src.getClientMaxBodySize()), \
_tryFiles(src.getTryFiles()), \
_errorPages(src.getErrorPages()) {}

LocationContext & LocationContext::operator=(const LocationContext & src) {
	_root = src.getRoot();
	_index = src.getIndex();
	_autoIndex = src.getAutoIndex();
	_clientMaxBodySize = src.getClientMaxBodySize();
	_tryFiles = src.getTryFiles();
	_errorPages = src.getErrorPages();
}

LocationContext::~LocationContext() {}

// Getters
std::string LocationContext::getRoot() const {
	return _root;
}

std::vector<std::string> LocationContext::getIndex() const {
	return _index;
}

State LocationContext::getAutoIndex() const {
	return _autoIndex;
}

long LocationContext::getClientMaxBodySize() const {
	return _clientMaxBodySize;
}

std::vector<std::string> LocationContext::getTryFiles() const {
	return _tryFiles;
}

std::vector<Method> LocationContext::getAllowedMethods() const {
	return _allowedMethods;
}

std::map<short, std::string> LocationContext::getErrorPages() const {
	return _errorPages;
}

// Setters
void LocationContext::setRoot(const std::string& root) {
	this->_root = root;
}

void LocationContext::setIndex(const std::vector<std::string>& index) {
	this->_index = index;
}

void LocationContext::setAutoIndex(State autoIndex) {
	this->_autoIndex = autoIndex;
}

void LocationContext::setClientMaxBodySize(long clientMaxBodySize) {
	this->_clientMaxBodySize = clientMaxBodySize;
}

void LocationContext::setTryFiles(const std::vector<std::string>& tryFiles) {
	this->_tryFiles = tryFiles;
}

void LocationContext::setAllowedMethods(const std::vector<Method>& allowedMethods) {
	this->_allowedMethods = allowedMethods;
}

void LocationContext::setErrorPages(const std::map<short, std::string>& errorPages) {
	this->_errorPages = errorPages;
}

// Methods to add elements
void LocationContext::addIndex(const std::string& index) {
	this->_index.push_back(index);
}

void LocationContext::addTryFile(const std::string& tryFile) {
	_tryFiles.push_back(tryFile);
}

void LocationContext::addAllowedMethod(const Method& method) {
	_allowedMethods.push_back(method);
}

void LocationContext::addErrorPage(short code, const std::string& page) {
	_errorPages[code] = page;
}
