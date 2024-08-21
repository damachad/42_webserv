/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerContext.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 11:37:41 by damachad          #+#    #+#             */
/*   Updated: 2024/08/21 14:21:27 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONTEXT_HPP
# define SERVERCONTEXT_HPP

# include "Webserv.hpp"

enum Method { GET = 1, POST, DELETE };

enum State {FALSE, TRUE, UNSET};

struct Listen {
	std::string IP;
	std::string port;
};

class LocationContext;

class ServerContext
{
	private:
		std::vector<Listen> _network_address;
		std::vector<std::string> _serverName;
		std::string _root;
		std::vector<std::string> _index;
		State _autoIndex;
		long _clientMaxBodySize;
		std::vector<std::string> _tryFiles;
		std::map<short, std::string> _errorPages;
		std::map<std::string, LocationContext> _locations;
		// std::string _uploadDir;	// Is this necessary ?
		// Later add redirect and cgi related variables
		typedef void (ServerContext::*DirectiveHandler)(
			std::vector<std::string> &);
		std::map<std::string, DirectiveHandler> _directiveMap;
	
	public:

		ServerContext();
		ServerContext(const ServerContext & src);
		~ServerContext();

		ServerContext & operator=(const ServerContext & src);

		// Getters
		std::vector<Listen> getNetworkAddress() const;
		std::vector<std::string> getServerName() const;
		std::string getRoot() const;
		std::vector<std::string> getIndex() const;
		State getAutoIndex() const;
		long getClientMaxBodySize() const;
		std::vector<std::string> getTryFiles() const;	
		std::map<short, std::string> getErrorPages() const;
		std::map<std::string, LocationContext> getLocations() const;

		// Setters
		void setNetworkAddress(const std::vector<Listen>& network_address);
		void setServerName(const std::vector<std::string>& serverName);
		void setRoot(const std::string& root);
		void setIndex(const std::vector<std::string>& index);
		void setAutoIndex(State autoIndex);
		void setClientMaxBodySize(long clientMaxBodySize);
		void setTryFiles(const std::vector<std::string>& tryFiles); // ?
		void setErrorPages(const std::map<short, std::string>& errorPages);
		void setLocations(const std::map<std::string, LocationContext>& locations);

		// Methods to add elements
		void addNetworkAddress(const Listen& address);
		void addServerName(const std::string& name);
		void addIndex(const std::string& index);
		void addTryFile(const std::string& tryFile);
		void addErrorPage(short code, const std::string& page);
		void addLocation(const std::string& path, const LocationContext& context);

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

std::ostream& operator<<(std::ostream& os, const ServerContext& context);

#endif