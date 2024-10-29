/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 11:37:41 by damachad          #+#    #+#             */
/*   Updated: 2024/10/16 11:57:20 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "ConfigParser.hpp"
#include "Exceptions.hpp"
#include "Webserv.hpp"

class Location {
   private:
	std::string _root;
	std::vector<std::string> _index;
	State _auto_index;
	long _client_max_body_size;
	std::set<Method> _allowed_methods;
	std::map<short, std::string> _error_pages;
	std::pair<short, std::string> _return;
	std::string _upload_store;
	std::string _cgi_ext;

	typedef void (Location::*DirectiveHandler)(std::vector<std::string> &);
	std::map<std::string, DirectiveHandler> _directive_map;

   public:
	Location();
	Location(const Location &src);
	~Location();

	Location &operator=(const Location &src);

	// Getters
	std::string getRoot() const;
	std::vector<std::string> getIndex() const;
	State getAutoIndex() const;
	long getClientMaxBodySize() const;
	std::set<Method> getAllowedMethods() const;
	std::map<short, std::string> getErrorPages() const;
	std::pair<short, std::string> getReturn() const;
	std::string getUpload() const;
	std::string getCgiExt() const;

	// Handlers for directives
	void handleRoot(std::vector<std::string> &tokens);
	void handleIndex(std::vector<std::string> &tokens);
	void handleLimitExcept(std::vector<std::string> &tokens);
	void handleErrorPage(std::vector<std::string> &tokens);
	void handleCliMaxSize(std::vector<std::string> &tokens);
	void handleAutoIndex(std::vector<std::string> &tokens);
	void handleReturn(std::vector<std::string> &tokens);
	void handleUpload(std::vector<std::string> &tokens);
	void handleCgiExt(std::vector<std::string> &tokens);

	void initializeDirectiveMap();
	void processDirective(std::string &line);
};

std::ostream &operator<<(std::ostream &os, const Location &context);

#endif
