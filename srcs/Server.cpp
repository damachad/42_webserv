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

Server::Server() : _autoIndex(FALSE), _clientMaxBodySize(1048576) {
	// NGINX defaults
	_index.push_back("index.html");
	_index.push_back("index.htm");
	initializeDirectiveMap();
	std::set<Method> methods;
	methods.insert(GET);
	methods.insert(POST);
	methods.insert(DELETE);
	_allowedMethods = methods;
}

Server::Server(const Server &src)
	: _network_address(src.getNetworkAddress()),
	  _serverName(src.getServerName()),
	  _root(src.getRoot()),
	  _index(src.getIndex()),
	  _autoIndex(src.getAutoIndex()),
	  _clientMaxBodySize(src.getClientMaxBodySize()),
	  _allowedMethods(src.getAllowedMethods()),
	  _errorPages(src.getErrorPages()),
	  _locations(src.getLocations()),
	  _return(src.getReturn()),
	  _uploadStore(src.getUpload()) {}

Server &Server::operator=(const Server &src) {
	_network_address = src.getNetworkAddress();
	_serverName = src.getServerName();
	_root = src.getRoot();
	_index = src.getIndex();
	_autoIndex = src.getAutoIndex();
	_clientMaxBodySize = src.getClientMaxBodySize();
	_allowedMethods = src.getAllowedMethods();
	_errorPages = src.getErrorPages();
	_locations = src.getLocations();
	_return = src.getReturn();
	_uploadStore = src.getUpload();
	return (*this);
}

Server::~Server() {}

// Function to initialize the map
void Server::initializeDirectiveMap(void) {
	_directiveMap["listen"] = &Server::handleListen;
	_directiveMap["server_name"] = &Server::handleServerName;
	_directiveMap["root"] = &Server::handleRoot;
	_directiveMap["index"] = &Server::handleIndex;
	_directiveMap["error_page"] = &Server::handleErrorPage;
	_directiveMap["client_max_body_size"] = &Server::handleCliMaxSize;
	_directiveMap["autoindex"] = &Server::handleAutoIndex;
	_directiveMap["return"] = &Server::handleReturn;
	_directiveMap["upload_store"] = &Server::handleUpload;
}

static bool isValidPort(const std::string &port) {
	for (std::string::size_type i = 0; i < port.size(); ++i) {
		if (!std::isdigit(port[i])) return false;
	}
	int portNumber = std::atoi(port.c_str());
	return (portNumber >= 0 && portNumber <= 65535);
}

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

// TODO: implement IPv6? default_server ?
// test what happens in NGINX address:<nothing> or <nothing>:port,
// is it the same as not including that parameter?
void Server::handleListen(std::vector<std::string> &tokens) {
	if (tokens.size() > 2)
		throw ConfigError("Too many arguments in listen directive.");
	std::vector<std::string>::const_iterator
		it;	 // check if accept just one token (if not implementing
			 // default_server)
	for (it = tokens.begin() + 1; it != tokens.end(); it++) {
		Listen listening;
		std::string value = (*it);
		size_t colonPos = value.find(':');
		if (colonPos != std::string::npos) {
			listening.IP = value.substr(0, colonPos);
			listening.port = value.substr(colonPos + 1);
			if (listening.IP.empty() || listening.port.empty())
				throw ConfigError("Invalid IP:Port.");
		} else {
			if ((*it).find_first_not_of("0123456789") == std::string::npos)
				listening.port = value;
			else
				listening.IP = value;
		}
		if (listening.IP.empty()) listening.IP = "";		// default
		if (listening.port.empty()) listening.port = "80";	// default
		if (!isValidIp(listening.IP) || !isValidPort(listening.port))
			throw ConfigError("Invalid IP:Port.");
		_network_address.push_back(listening);
	}
}

void Server::handleServerName(std::vector<std::string> &tokens) {
	tokens.erase(tokens.begin());
	_serverName = tokens;
}

void Server::handleRoot(std::vector<std::string> &tokens) {
	if (tokens.size() > 2)
		throw ConfigError("Too many arguments in root directive.");
	_root = tokens[1];
}

void Server::handleIndex(std::vector<std::string> &tokens) {
	tokens.erase(tokens.begin());
	_index = tokens;
}

void Server::handleErrorPage(std::vector<std::string> &tokens) {
	if (tokens.size() < 3) throw ConfigError("Invalid error_page directive.");
	std::string page = tokens.back();
	for (size_t i = 1; i < tokens.size() - 1; i++) {
		char *end;
		long statusCodeLong = std::strtol(tokens[i].c_str(), &end, 10);
		if (*end != '\0' || statusCodeLong < 300 || statusCodeLong > 599 ||
			statusCodeLong != static_cast<short>(statusCodeLong))
			throw ConfigError("Invalid status code in error_page directive.");
		_errorPages[static_cast<short>(statusCodeLong)] = page;
	}
}

void Server::handleCliMaxSize(std::vector<std::string> &tokens) {
	if (tokens.size() != 2)
		throw ConfigError("Invalid client_max_body_size directive.");
	std::string maxSize = tokens[1];
	char unit = maxSize[maxSize.size() - 1];
	if (!std::isdigit(unit)) maxSize.resize(maxSize.size() - 1);

	// check if there is overflow
	char *endPtr = NULL;
	long size = std::strtol(maxSize.c_str(), &endPtr, 10);
	if (*endPtr != '\0')
		throw ConfigError("Invalid numeric value for client_max_body_size.");
	_clientMaxBodySize = size;
	if (!std::isdigit(unit)) {
		// check for overflow during multiplication
		const long maxLimit = LONG_MAX;
		switch (unit) {
			case 'k':
			case 'K':
				if (size > maxLimit / 1024)
					throw ConfigError("client_max_body_size value overflow.");
				_clientMaxBodySize = size * 1024;
				break;
			case 'm':
			case 'M':
				if (size > maxLimit / 1048576)
					throw ConfigError("client_max_body_size value overflow.");
				_clientMaxBodySize = size * 1048576;
				break;
			case 'g':
			case 'G':
				if (size > maxLimit / 1073741824)
					throw ConfigError("client_max_body_size value overflow.");
				_clientMaxBodySize = size * 1073741824;
				break;
			default:
				throw ConfigError("Invalid unit for client_max_body_size.");
		}
	}
}

void Server::handleAutoIndex(std::vector<std::string> &tokens) {
	if (tokens[1] == "on")
		_autoIndex = TRUE;
	else if (tokens[1] == "off")
		_autoIndex = FALSE;
	else
		throw ConfigError("Invalid syntax.");
}

void Server::handleReturn(std::vector<std::string> &tokens) {
	if (tokens.size() != 3) throw ConfigError("Invalid return directive.");
	// check if there is overflow
	char *endPtr = NULL;
	long errorCode = std::strtol(tokens[1].c_str(), &endPtr, 10);
	if (*endPtr != '\0' || errorCode < 0 || errorCode > 999 ||
		errorCode != static_cast<short>(errorCode))	 // accepted NGINX values
		throw ConfigError("Invalid error code for return directive.");
	if (_return.first) return;
	_return.first = static_cast<short>(errorCode);
	_return.second = tokens[2];
}

void Server::handleUpload(std::vector<std::string> &tokens) {
	if (tokens.size() != 2) throw ConfigError("Invalid upload_store directive.");
	_uploadStore = tokens[1];
}

void Server::processDirective(std::string &line) {
	std::vector<std::string> tokens;
	tokens = ConfigParser::tokenizeLine(line);
	if (tokens.size() < 2)
		throw ConfigError("No value for directive: " + tokens[0]);
	std::map<std::string, DirectiveHandler>::const_iterator it;
	it = _directiveMap.find(tokens[0]);
	if (it != _directiveMap.end())
		(this->*(it->second))(tokens);
	else
		throw ConfigError("Unkown directive: " + tokens[0]);
}

// TODO: improve flow of function
//		load inherited values?
void Server::processLocation(std::string block, size_t start, size_t end) {
	std::string route;
	std::string line;
	std::string firstWord;
	std::vector<std::string> tokens;
	std::istringstream location(block.substr(start, end - start));
	LocationContext locationInfo;

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

std::vector<std::string> Server::getServerName() const { return _serverName; }

std::string Server::getRoot() const { return _root; }

std::vector<std::string> Server::getIndex() const { return _index; }

State Server::getAutoIndex() const { return _autoIndex; }

long Server::getClientMaxBodySize() const { return _clientMaxBodySize; }

std::set<Method> Server::getAllowedMethods() const { return _allowedMethods; }

std::map<short, std::string> Server::getErrorPages() const {
	return _errorPages;
}

std::map<std::string, LocationContext> Server::getLocations() const {
	return _locations;
}

std::pair<short, std::string> Server::getReturn() const { return _return; }

std::string Server::getUpload() const {
	return _uploadStore;
}

std::string Server::getRoot(const std::string &route) const {
	if (route.empty()) return _root;
	std::map<std::string, LocationContext>::const_iterator it;
	it = _locations.find(route);
	if (it == _locations.end() || it->second.getRoot().empty())
		return _root;
	else
		return it->second.getRoot();
}

std::vector<std::string> Server::getIndex(const std::string &route) const {
	if (route.empty()) return _index;
	std::map<std::string, LocationContext>::const_iterator it;
	it = _locations.find(route);
	if (it == _locations.end() || it->second.getIndex().empty())
		return _index;
	else
		return it->second.getIndex();
}

State Server::getAutoIndex(const std::string &route) const {
	if (route.empty()) return _autoIndex;
	std::map<std::string, LocationContext>::const_iterator it;
	it = _locations.find(route);
	if (it == _locations.end() || it->second.getAutoIndex() == UNSET)
		return _autoIndex;
	else
		return it->second.getAutoIndex();
}

long Server::getClientMaxBodySize(const std::string &route) const {
	if (route.empty()) return _clientMaxBodySize;
	std::map<std::string, LocationContext>::const_iterator it;
	it = _locations.find(route);
	if (it == _locations.end() || it->second.getClientMaxBodySize() == -1)
		return _clientMaxBodySize;
	else
		return it->second.getClientMaxBodySize();
}

std::map<short, std::string> Server::getErrorPages(
	const std::string &route) const {
	if (route.empty()) return _errorPages;
	std::map<std::string, LocationContext>::const_iterator it;
	it = _locations.find(route);
	if (it == _locations.end() || it->second.getErrorPages().empty())
		return _errorPages;
	else
		return it->second.getErrorPages();
}

std::set<Method> Server::getAllowedMethods(const std::string &route) const {
	if (route.empty()) return _allowedMethods;
	std::map<std::string, LocationContext>::const_iterator it;
	it = _locations.find(route);
	if (it == _locations.end() || it->second.getAllowedMethods().empty())
		return _allowedMethods;
	else
		return it->second.getAllowedMethods();
}

std::pair<short, std::string> Server::getReturn(
	const std::string &route) const {
	if (route.empty()) return _return;
	std::map<std::string, LocationContext>::const_iterator it;
	it = _locations.find(route);
	if (it == _locations.end() || it->second.getReturn().first == 0)
		return _return;
	else
		return it->second.getReturn();
}

std::string Server::getUpload(const std::string &route) const {
	if (route.empty()) return _uploadStore;
	std::map<std::string, LocationContext>::const_iterator it;
	it = _locations.find(route);
	if (it == _locations.end() || it->second.getUpload().empty())
		return _uploadStore;
	else
		return it->second.getUpload();
}

std::vector<int> Server::getListeningSockets(void) const {
	return _listening_sockets;
}

void Server::setup_server(void) {
	const std::vector<Listen> network_addresses = getNetworkAddress();

	for (std::vector<Listen>::const_iterator it = network_addresses.begin();
		 it != network_addresses.end(); it++) {
		// Create a socket (IPv4, TCP)
		int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (sock_fd == -1) throw SocketSetupError("socket");

		// Set the socket option SO_REUSEADDR
		// NOTE: Allows for quicker debugging because socket doesn't get held
		// TODO: REMOVE AT THE END????
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
	if (returns.first)
		os << "    " << returns.first << " : " << returns.second << "\n";

	os << "Locations:\n";
	std::map<std::string, LocationContext> locations = context.getLocations();
	for (std::map<std::string, LocationContext>::const_iterator it =
			 locations.begin();
		 it != locations.end(); ++it) {
		os << "  " << it->first << " :\n" << it->second << "\n";
	}
	os << "  Upload Store: " << context.getUpload() << "\n";

	return os;
}
