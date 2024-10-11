/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 11:37:41 by damachad          #+#    #+#             */
/*   Updated: 2024/08/29 14:13:58 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

class LocationContext;

#include "LocationContext.hpp"
#include "Webserv.hpp"

struct Listen {
	std::string IP;
	std::string port;
};

class Server {
   private:
	// Server Context Data
	std::vector<Listen> _network_address;
	std::vector<std::string> _serverName;
	std::string _root;
	std::vector<std::string> _index;
	State _autoIndex;
	long _clientMaxBodySize;
	std::set<Method> _allowedMethods;
	std::map<short, std::string> _errorPages;
	std::map<std::string, LocationContext> _locations;
	std::pair<short, std::string> _return;
	std::string _uploadStore;
	// Later add cgi related variables
	typedef void (Server::*DirectiveHandler)(std::vector<std::string> &);
	std::map<std::string, DirectiveHandler> _directiveMap;

	// Connection data
	std::vector<int> _listening_sockets;
	std::vector<struct sockaddr_in> _sockaddr_vector;

   public:
	Server();
	Server(const Server &src);
	~Server();

	Server &operator=(const Server &src);

	// Getters
	std::vector<Listen> getNetworkAddress() const;
	std::vector<std::string> getServerName() const;
	std::string getRoot() const;
	std::vector<std::string> getIndex() const;
	State getAutoIndex() const;
	long getClientMaxBodySize() const;
	std::set<Method> getAllowedMethods() const;
	std::map<short, std::string> getErrorPages() const;
	std::map<std::string, LocationContext> getLocations() const;
	std::pair<short, std::string> getReturn() const;
	std::string getUpload() const;

	std::string getRoot(const std::string &route) const;
	std::vector<std::string> getIndex(const std::string &route) const;
	State getAutoIndex(const std::string &route) const;
	long getClientMaxBodySize(const std::string &route) const;
	std::map<short, std::string> getErrorPages(const std::string &route) const;
	std::set<Method> getAllowedMethods(const std::string &route) const;
	std::pair<short, std::string> getReturn(const std::string &route) const;
	std::string getUpload(const std::string &route) const;

	std::vector<int> getListeningSockets(void) const;

	// Handlers for directives
	void handleListen(std::vector<std::string> &tokens);
	void handleServerName(std::vector<std::string> &tokens);
	void handleRoot(std::vector<std::string> &tokens);
	void handleIndex(std::vector<std::string> &tokens);
	void handleErrorPage(std::vector<std::string> &tokens);
	void handleCliMaxSize(std::vector<std::string> &tokens);
	void handleAutoIndex(std::vector<std::string> &tokens);
	void handleReturn(std::vector<std::string> &tokens);
	void handleUpload(std::vector<std::string> &tokens);

	void initializeDirectiveMap();
	void processDirective(std::string &line);
	void processLocation(std::string block, size_t start, size_t end);

	// Server setup function
	void setup_server(void);
};

std::ostream &operator<<(std::ostream &os, const Server &context);

#endif
