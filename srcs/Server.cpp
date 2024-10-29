/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 11:47:36 by damachad          #+#    #+#             */
/*   Updated: 2024/09/09 17:22:49 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server() : _auto_index(FALSE), _client_max_body_size(1048576) {
	// NGINX defaults
	_index.push_back("index.html");
	_index.push_back("index.htm");
	initializeDirectiveMap();
	std::set<Method> methods;
	methods.insert(GET);
	methods.insert(POST);
	methods.insert(DELETE);
	_allowed_methods = methods;
	_return = std::make_pair(-1, "");
}

Server::Server(const Server &src)
	: _network_address(src.getNetworkAddress()),
	  _server_name(src.getServerName()),
	  _root(src.getRoot()),
	  _index(src.getIndex()),
	  _auto_index(src.getAutoIndex()),
	  _client_max_body_size(src.getClientMaxBodySize()),
	  _allowed_methods(src.getAllowedMethods()),
	  _error_pages(src.getErrorPages()),
	  _locations(src.getLocations()),
	  _return(src.getReturn()),
	  _upload_store(src.getUpload()),
	  _cgi_ext(src.getCgiExt()) {}

Server &Server::operator=(const Server &src) {
	_network_address = src.getNetworkAddress();
	_server_name = src.getServerName();
	_root = src.getRoot();
	_index = src.getIndex();
	_auto_index = src.getAutoIndex();
	_client_max_body_size = src.getClientMaxBodySize();
	_allowed_methods = src.getAllowedMethods();
	_error_pages = src.getErrorPages();
	_locations = src.getLocations();
	_return = src.getReturn();
	_upload_store = src.getUpload();
	_cgi_ext = src.getCgiExt();
	return (*this);
}

Server::~Server() {}

// Function to initialize directive map
void Server::initializeDirectiveMap(void) {
	_directive_map["listen"] = &Server::handleListen;
	_directive_map["server_name"] = &Server::handleServerName;
	_directive_map["root"] = &Server::handleRoot;
	_directive_map["index"] = &Server::handleIndex;
	_directive_map["error_page"] = &Server::handleErrorPage;
	_directive_map["client_max_body_size"] = &Server::handleCliMaxSize;
	_directive_map["autoindex"] = &Server::handleAutoIndex;
	_directive_map["return"] = &Server::handleReturn;
	_directive_map["upload_store"] = &Server::handleUpload;
	_directive_map["cgi_ext"] = &Server::handleCgiExt;
}

// Checks if a port is valid
static bool isValidPort(const std::string &port) {
	for (std::string::size_type i = 0; i < port.size(); ++i) {
		if (!std::isdigit(port[i])) return false;
	}
	int portNumber = std::atoi(port.c_str());
	return (portNumber >= 0 && portNumber <= 65535);
}

// Checks if an IP is valid
static bool isValidIp(const std::string &ip) {
	if (ip.empty() || ip == "localhost") return true;
	int numDots = 0;
	std::string segment;
	std::istringstream ipStream(ip);

	while (std::getline(ipStream, segment, '.')) {
		if (segment.empty() || segment.size() > 3) return false;
		for (std::string::size_type i = 0; i < segment.size(); i++)
			if (!std::isdigit(segment[i])) return false;
		int num = std::atoi(segment.c_str());
		if (num < 0 || num > 255) return false;
		numDots++;
	}
	return (numDots == 4);
}

// Handlers

// Checks and loads listen information into Server
void Server::handleListen(std::vector<std::string> &tokens) {
	if (tokens.size() > 2)
		throw ConfigError("Invalid number of arguments in listen directive.");
	std::vector<std::string>::const_iterator it;
	for (it = tokens.begin() + 1; it != tokens.end(); it++) {
		Listen listening;
		std::string value = (*it);
		size_t colonPos = value.find(':');
		if (colonPos != std::string::npos) {
			listening.IP = value.substr(0, colonPos);
			listening.port = value.substr(colonPos + 1);
			if (listening.IP.empty() || listening.port.empty())
				throw ConfigError("Invalid IP:Port in listen directive.");
		} else {
			if ((*it).find_first_not_of("0123456789") == std::string::npos)
				listening.port = value;
			else
				listening.IP = value;
		}
		if (listening.IP.empty()) listening.IP = "";		// default
		if (listening.port.empty()) listening.port = "80";	// default
		if (!isValidIp(listening.IP) || !isValidPort(listening.port))
			throw ConfigError("Invalid IP:Port in listen directive.");
		_network_address.push_back(listening);
	}
}

// Loads server_name into Server
void Server::handleServerName(std::vector<std::string> &tokens) {
	tokens.erase(tokens.begin());
	_server_name = tokens;
}

// Checks and loads root into Server
void Server::handleRoot(std::vector<std::string> &tokens) {
	if (tokens.size() > 2)
		throw ConfigError("Invalid number of arguments in root directive.");
	_root = tokens[1];
}

// Loads index into Server
void Server::handleIndex(std::vector<std::string> &tokens) {
	tokens.erase(tokens.begin());
	_index = tokens;
}

// Checks and loads error_page into Server
void Server::handleErrorPage(std::vector<std::string> &tokens) {
	if (tokens.size() < 3)
		throw ConfigError(
			"Invalid number of arguments in error_page directive.");
	std::string page = tokens.back();
	for (size_t i = 1; i < tokens.size() - 1; i++) {
		char *end;
		long statusCodeLong = std::strtol(tokens[i].c_str(), &end, 10);
		if (*end != '\0' || statusCodeLong < 300 || statusCodeLong > 599 ||
			statusCodeLong != static_cast<short>(statusCodeLong))
			throw ConfigError("Invalid status code in error_page directive.");
		_error_pages[static_cast<short>(statusCodeLong)] = page;
	}
}

// Checks and loads client_max_body_size into Server
void Server::handleCliMaxSize(std::vector<std::string> &tokens) {
	if (tokens.size() != 2)
		throw ConfigError(
			"Invalid number of arguments in client_max_body_size directive.");
	std::string maxSize = tokens[1];
	char unit = maxSize[maxSize.size() - 1];
	if (!std::isdigit(unit)) maxSize.resize(maxSize.size() - 1);

	// check if there is overflow
	char *endPtr = NULL;
	long size = std::strtol(maxSize.c_str(), &endPtr, 10);
	if (*endPtr != '\0')
		throw ConfigError("Invalid value for client_max_body_size.");
	_client_max_body_size = size;
	if (!std::isdigit(unit)) {
		// check for overflow during multiplication
		const long maxLimit = LONG_MAX;
		switch (unit) {
			case 'k':
			case 'K':
				if (size > maxLimit / 1024)
					throw ConfigError("client_max_body_size value overflow.");
				_client_max_body_size = size * 1024;
				break;
			case 'm':
			case 'M':
				if (size > maxLimit / 1048576)
					throw ConfigError("client_max_body_size value overflow.");
				_client_max_body_size = size * 1048576;
				break;
			case 'g':
			case 'G':
				if (size > maxLimit / 1073741824)
					throw ConfigError("client_max_body_size value overflow.");
				_client_max_body_size = size * 1073741824;
				break;
			default:
				throw ConfigError("Invalid unit for client_max_body_size.");
		}
	}
}

// Checks and loads autoindex into Server
void Server::handleAutoIndex(std::vector<std::string> &tokens) {
	if (tokens[1] == "on")
		_auto_index = TRUE;
	else if (tokens[1] == "off")
		_auto_index = FALSE;
	else
		throw ConfigError("Invalid value in autoindex directive.");
}

// Checks and loads return into Server
void Server::handleReturn(std::vector<std::string> &tokens) {
	if (tokens.size() != 3)
		throw ConfigError("Invalid number of arguments in return directive.");
	// check if there is overflow
	char *endPtr = NULL;
	long errorCode = std::strtol(tokens[1].c_str(), &endPtr, 10);
	if (*endPtr != '\0' || errorCode < 0 || errorCode > 999 ||
		errorCode != static_cast<short>(errorCode))	 // accepted NGINX values
		throw ConfigError("Invalid error code for return directive.");
	if (_return.first != -1) return;
	_return.first = static_cast<short>(errorCode);
	_return.second = tokens[2];
}

// Checks and loads upload into Server
void Server::handleUpload(std::vector<std::string> &tokens) {
	if (tokens.size() != 2)
		throw ConfigError(
			"Invalid number of arguments in upload_store directive.");
	_upload_store = tokens[1];
}

// Checks and loads cgi_ext into Server
void Server::handleCgiExt(std::vector<std::string> &tokens) {
	if (tokens.size() > 2)
		throw ConfigError("Invalid number of arguments in cgi_ext directive.");
	_cgi_ext = tokens[1];
}

// Checks directive map and calls corresponding function, if it exists
void Server::processDirective(std::string &line) {
	std::vector<std::string> tokens;
	tokens = ConfigParser::tokenizeLine(line);
	if (tokens.size() < 2)
		throw ConfigError("Invalid number of arguments in \"" + tokens[0] +
						  "\" directive.");
	std::map<std::string, DirectiveHandler>::const_iterator it;
	it = _directive_map.find(tokens[0]);
	if (it != _directive_map.end())
		(this->*(it->second))(tokens);
	else
		throw ConfigError("Unkown directive \"" + tokens[0] + "\"");
}

// Loads a location block and adds it to the map in server
void Server::processLocation(std::string block, size_t start, size_t end) {
	std::string route;
	std::string line;
	std::string firstWord;
	std::vector<std::string> tokens;
	std::istringstream location(block.substr(start, end - start));
	Location locationInfo;

	location >> route;	// Discard 'location'
	location >> route;	// Get the actual route
	if (route == "{") throw ConfigError("No location route.");
	location >> line;  // Discard the opening '{'
	if (line != "{") throw ConfigError("Location can only support one route.");
	while (std::getline(location, line, ';')) {
		ConfigParser::trimOuterSpaces(line);
		if ((line.empty() && !location.eof()))
			throw ConfigError("Unparsable location block detected.");
		if (line.empty()) continue;
		std::istringstream readLine(line);
		readLine >> firstWord;
		locationInfo.processDirective(line);
		if (location.tellg() >= static_cast<std::streampos>(end)) break;
	}
	_locations[route] = locationInfo;
}

// Getters
std::vector<Listen> Server::getNetworkAddress() const {
	return _network_address;
}

std::vector<std::string> Server::getServerName() const { return _server_name; }

std::string Server::getRoot() const { return _root; }

std::vector<std::string> Server::getIndex() const { return _index; }

State Server::getAutoIndex() const { return _auto_index; }

long Server::getClientMaxBodySize() const { return _client_max_body_size; }

std::set<Method> Server::getAllowedMethods() const { return _allowed_methods; }

std::map<short, std::string> Server::getErrorPages() const {
	return _error_pages;
}

std::map<std::string, Location> Server::getLocations() const {
	return _locations;
}

std::pair<short, std::string> Server::getReturn() const { return _return; }

std::string Server::getUpload() const { return _upload_store; }

std::string Server::getCgiExt() const { return _cgi_ext; }

std::string Server::getRoot(const std::string &route) const {
	if (route.empty()) return _root;
	std::map<std::string, Location>::const_iterator it;
	it = _locations.find(route);
	if (it == _locations.end() || it->second.getRoot().empty())
		return _root;
	else
		return it->second.getRoot();
}

std::vector<std::string> Server::getIndex(const std::string &route) const {
	if (route.empty()) return _index;
	std::map<std::string, Location>::const_iterator it;
	it = _locations.find(route);
	if (it == _locations.end() || it->second.getIndex().empty())
		return _index;
	else
		return it->second.getIndex();
}

State Server::getAutoIndex(const std::string &route) const {
	if (route.empty()) return _auto_index;
	std::map<std::string, Location>::const_iterator it;
	it = _locations.find(route);
	if (it == _locations.end() || it->second.getAutoIndex() == UNSET)
		return _auto_index;
	else
		return it->second.getAutoIndex();
}

long Server::getClientMaxBodySize(const std::string &route) const {
	if (route.empty()) return _client_max_body_size;
	std::map<std::string, Location>::const_iterator it;
	it = _locations.find(route);
	if (it == _locations.end() || it->second.getClientMaxBodySize() == -1)
		return _client_max_body_size;
	else
		return it->second.getClientMaxBodySize();
}

std::map<short, std::string> Server::getErrorPages(
	const std::string &route) const {
	if (route.empty()) return _error_pages;
	std::map<std::string, Location>::const_iterator it;
	it = _locations.find(route);
	if (it == _locations.end() || it->second.getErrorPages().empty())
		return _error_pages;
	else
		return it->second.getErrorPages();
}

std::set<Method> Server::getAllowedMethods(const std::string &route) const {
	if (route.empty()) return _allowed_methods;
	std::map<std::string, Location>::const_iterator it;
	it = _locations.find(route);
	if (it == _locations.end() || it->second.getAllowedMethods().empty())
		return _allowed_methods;
	else
		return it->second.getAllowedMethods();
}

std::pair<short, std::string> Server::getReturn(
	const std::string &route) const {
	if (route.empty()) return _return;
	std::map<std::string, Location>::const_iterator it;
	it = _locations.find(route);
	if (it == _locations.end() || it->second.getReturn().first == -1)
		return _return;
	else
		return it->second.getReturn();
}

std::string Server::getUpload(const std::string &route) const {
	if (route.empty()) return _upload_store;
	std::map<std::string, Location>::const_iterator it;
	it = _locations.find(route);
	if (it == _locations.end() || it->second.getUpload().empty())
		return _upload_store;
	else
		return it->second.getUpload();
}

std::string Server::getCgiExt(const std::string &route) const {
	if (route.empty()) return _cgi_ext;
	std::map<std::string, Location>::const_iterator it;
	it = _locations.find(route);
	if (it == _locations.end() || it->second.getCgiExt().empty())
		return _cgi_ext;
	else
		return it->second.getCgiExt();
}

std::vector<int> Server::getListeningSockets(void) const {
	return _listening_sockets;
}

// Sets up sockets based on servers
void Server::setupServer(void) {
	const std::vector<Listen> network_addresses = getNetworkAddress();

	for (std::vector<Listen>::const_iterator it = network_addresses.begin();
		 it != network_addresses.end(); it++) {
		// Create a socket (IPv4, TCP)
		int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (sock_fd == -1) throw SocketSetupError("socket");

		// Set the socket option SO_REUSEADDR
		// NOTE: Allows for quicker debugging because socket doesn't get held
		int optval = 1;
		if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &optval,
					   sizeof(optval)) < 0) {
			std::cerr << "Error: Could not set SO_REUSEADDR\n";
			return;
		}

		// Listen to connections on socket (port given by *it)
		struct sockaddr_in sockaddr;
		std::memset(&sockaddr, 0, sizeof(sockaddr));  // Clears the struct
		sockaddr.sin_family = AF_INET;	// IPv4 Internet Protocolos
		if (it->IP == "")
			sockaddr.sin_addr.s_addr =
				INADDR_ANY;	 // Binds to all available interfaces
		else if (it->IP == "localhost") {
			if (inet_aton("127.0.0.1", &sockaddr.sin_addr) == 0)
				throw SocketSetupError("inet_addr");

		} else {
			if (inet_aton(it->IP.c_str(), &sockaddr.sin_addr) == 0)
				throw SocketSetupError("inet_addr");
		}
		sockaddr.sin_port = htons(stringToNumber<int>(
			it->port));	 // Converts number to network byte order

		// Binds name to socket
		if (bind(sock_fd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0) {
			close(sock_fd);
			throw SocketSetupError("bind");
		}

		// Starts listening to incoming connections
		if (listen(sock_fd, SOMAXCONN) < 0) {
			close(sock_fd);
			throw SocketSetupError("listen");
		}

		// Adds sock_fd and sockaddr to the server object. NOTE: Unneeded?
		_listening_sockets.push_back(sock_fd);
		_sockaddr_vector.push_back(sockaddr);
	}
}

// Sends Server information into ostream
std::ostream &operator<<(std::ostream &os, const Server &context) {
	os << "Network Addresses:\n";
	std::vector<Listen> networkAddress = context.getNetworkAddress();
	for (std::vector<Listen>::const_iterator it = networkAddress.begin();
		 it != networkAddress.end(); ++it)
		os << "  IP: " << it->IP << ", Port: " << it->port << "\n";

	os << "Server Names:\n";
	std::vector<std::string> serverNames = context.getServerName();
	for (std::vector<std::string>::const_iterator it = serverNames.begin();
		 it != serverNames.end(); ++it)
		os << "  " << *it << "\n";

	os << "Root: " << context.getRoot() << "\n";

	os << "Index Files:\n";
	std::vector<std::string> indexFiles = context.getIndex();
	for (std::vector<std::string>::const_iterator it = indexFiles.begin();
		 it != indexFiles.end(); ++it) {
		os << "  " << *it << "\n";
	}

	os << "AutoIndex: "
	   << (context.getAutoIndex() == TRUE	 ? "TRUE"
		   : context.getAutoIndex() == FALSE ? "FALSE"
											 : "UNSET")
	   << "\n";

	os << "Client Max Body Size: " << context.getClientMaxBodySize() << "\n";

	os << "Error Pages:\n";
	std::map<short, std::string> errorPages = context.getErrorPages();
	for (std::map<short, std::string>::const_iterator it = errorPages.begin();
		 it != errorPages.end(); ++it) {
		os << "  " << it->first << " : " << it->second << "\n";
	}

	os << "Return:\n";
	std::pair<short, std::string> returns = context.getReturn();
	if (returns.first != -1)
		os << "    " << returns.first << " : " << returns.second << "\n";

	os << "Locations:\n";
	std::map<std::string, Location> locations = context.getLocations();
	for (std::map<std::string, Location>::const_iterator it = locations.begin();
		 it != locations.end(); ++it) {
		os << "  " << it->first << " :\n" << it->second << "\n";
	}
	os << "  Upload Store: " << context.getUpload() << "\n";
	os << "  CGI Extension: " << context.getCgiExt() << "\n";

	return os;
}
