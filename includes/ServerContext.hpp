/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerContext.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 11:37:41 by damachad          #+#    #+#             */
/*   Updated: 2024/08/21 16:10:04 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONTEXT_HPP
#define SERVERCONTEXT_HPP

#include "Webserv.hpp"

enum Method { GET = 1, POST, DELETE };

enum State { FALSE, TRUE, UNSET };

struct Listen {
	std::string IP;
	std::string port;
};

class LocationContext;

class ServerContext {
   private:
	std::vector<Listen> _network_address;
	std::vector<std::string> _serverName;
	std::string _root;
	std::vector<std::string> _index;
	State _autoIndex;
	long _clientMaxBodySize;
	std::vector<std::string> _tryFiles;
	std::vector<Method> _allowedMethods;
	std::map<short, std::string> _errorPages;
	std::map<std::string, LocationContext> _locations;
	// std::string _uploadDir;	// Is this necessary ?
	// Later add redirect and cgi related variables
	typedef void (ServerContext::*DirectiveHandler)(std::vector<std::string> &);
	std::map<std::string, DirectiveHandler> _directiveMap;

   public:
	ServerContext();
	ServerContext(const ServerContext &src);
	~ServerContext();

	ServerContext &operator=(const ServerContext &src);

	// Getters
	std::vector<Listen> getNetworkAddress() const;
	std::vector<std::string> getServerName() const;
	std::string getRoot() const;
	std::vector<std::string> getIndex() const;
	State getAutoIndex() const;
	long getClientMaxBodySize() const;
	std::vector<std::string> getTryFiles() const;
	std::vector<Method> getAllowedMethods() const;
	std::map<short, std::string> getErrorPages() const;
	std::map<std::string, LocationContext> getLocations() const;

	std::string getRoot(const std::string &route) const;
	std::vector<std::string> getIndex(const std::string &route) const;
	State getAutoIndex(const std::string &route) const;
	long getClientMaxBodySize(const std::string &route) const;
	std::vector<std::string> getTryFiles(const std::string &route) const;
	std::map<short, std::string> getErrorPages(const std::string &route) const;
	std::vector<Method> getAllowedMethods(const std::string &route) const;

	// Handlers for directives
	void handleListen(std::vector<std::string> &tokens);
	void handleServerName(std::vector<std::string> &tokens);
	void handleRoot(std::vector<std::string> &tokens);
	void handleIndex(std::vector<std::string> &tokens);
	void handleTryFiles(std::vector<std::string> &tokens);
	void handleErrorPage(std::vector<std::string> &tokens);
	void handleCliMaxSize(std::vector<std::string> &tokens);
	void handleAutoIndex(std::vector<std::string> &tokens);

	void initializeDirectiveMap();
	void processDirective(std::string &line);
	void processLocation(std::string block, size_t start, size_t end);
};

std::ostream &operator<<(std::ostream &os, const ServerContext &context);

#endif

