/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cluster.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/19 14:44:19 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/09/30 16:29:01 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cluster.hpp"

#include <netdb.h>
#include <sys/socket.h>

#include <utility>

#include "AResponse.hpp"
#include "DeleteResponse.hpp"
#include "GetResponse.hpp"
#include "HTTPRequestParser.hpp"
#include "Helpers.hpp"
#include "PostResponse.hpp"
#include "RequestErrorResponse.hpp"

// Constructor
// Create a vector of servers from provided context vector
Cluster::Cluster(const std::vector<Server>& servers)
	: _servers(), _epoll_fd(-1) {
	for (std::vector<Server>::const_iterator it = servers.begin();
		 it != servers.end(); it++) {
		(_servers).push_back(&(*it));

		std::vector<Listen> network_addresses = (*it).getNetworkAddress();
		std::vector<std::string> server_names = (*it).getServerName();

		for (std::vector<Listen>::const_iterator listen_it =
				 network_addresses.begin();
			 listen_it != network_addresses.end(); listen_it++) {
			if (server_names.empty()) {
				struct VirtualServer VServer;
				VServer.IP = listen_it->IP;
				VServer.port = listen_it->port;
				VServer.server_name = "";
				VServer.server = &(*it);
				_virtual_servers.push_back(VServer);
			} else {
				for (std::vector<std::string>::const_iterator servername_it =
						 server_names.begin();
					 servername_it != server_names.end(); servername_it++) {
					struct VirtualServer VServer;
					VServer.IP = listen_it->IP;
					VServer.port = listen_it->port;
					VServer.server_name = *servername_it;
					VServer.server = &(*it);
					_virtual_servers.push_back(VServer);
				}
			}
		}
	}
}

// Destructor
// Closes _epoll_fd if it was open
Cluster::~Cluster() {
	//	std::cout << "Cluster Destructor called" << std::endl;
	if (_epoll_fd >= 0) close(_epoll_fd);
}

// Accesses ith server of _server array when asking Cluster[i]
const Server& Cluster::operator[](unsigned int index) const {
	if (index >= _servers.size())
		throw OutOfBoundsError(numberToString<int>(index));

	return *_servers[index];
}

// Accesses ith server of _server array when asking Cluster[i]
bool Cluster::hasDuplicateVirtualServers() const {
	std::set<VirtualServer> seen;

	for (std::vector<VirtualServer>::const_iterator it =
			 _virtual_servers.begin();
		 it != _virtual_servers.end(); ++it) {
		std::pair<std::set<VirtualServer>::iterator, bool> result =
			seen.insert(*it);

		if (!result.second) {
			return true;
		}
	}

	return false;
}

// Creates epoll instance
void Cluster::createEpollInstance(void) {
	_epoll_fd = epoll_create(1);  // Int is ignored in newer implementations
	if (_epoll_fd == -1) throw ClusterSetupError("epoll_create");
}

// Creates sockets and binds to them
int Cluster::createAndBindSocket(const std::string& IP,
								 const std::string& port) {
	// Create a socket (IPv4, TCP)
	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd == -1) throw SocketSetupError("socket");

	// Set the socket option SO_REUSEADDR
	int optval = 1;
	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) <
		0)
		throw SocketSetupError("Could not set SO_REUSEADDR");

	// Listen to connections on socket
	struct sockaddr_in sockaddr;
	std::memset(&sockaddr, 0,
				sizeof(sockaddr));	// Clears the struct
	sockaddr.sin_family = AF_INET;	// IPv4 Internet Protocolos
	if (IP == "")
		sockaddr.sin_addr.s_addr =
			INADDR_ANY;	 // Binds to all available interfaces
	else if (IP == "localhost") {
		if (inet_aton("127.0.0.1", &sockaddr.sin_addr) == 0)
			throw SocketSetupError("inet_addr");

	} else {
		if (inet_aton(IP.c_str(), &sockaddr.sin_addr) == 0)
			throw SocketSetupError("inet_addr");
	}
	sockaddr.sin_port = htons(
		stringToNumber<int>(port));	 // Converts number to network byte order

	// Binds to socket
	if (bind(sock_fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
		close(sock_fd);
		throw SocketSetupError("bind");
	};

	return sock_fd;
}

// Starts listening to incoming connections
void Cluster::startListening(int sock_fd) {
	if (listen(sock_fd, SOMAXCONN) < 0) {
		close(sock_fd);
		throw SocketSetupError("listen");
	}
}

// Adds sockets to epoll so they can be monitored
void Cluster::addSocketsToEpoll(int sock_fd) {
	epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = sock_fd;

	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, sock_fd, &event) == -1)
		throw ClusterSetupError("epoll_ctl");
}

// Gets list of sockets needed
std::set<Listen> Cluster::trimVirtualServers() {
	std::vector<VirtualServer> virtual_server_copy = _virtual_servers;

	std::set<std::string> ports_to_remove;

	for (std::vector<VirtualServer>::const_iterator vs_it =
			 virtual_server_copy.begin();
		 vs_it != virtual_server_copy.end(); vs_it++) {
		if (vs_it->IP.empty()) ports_to_remove.insert(vs_it->port);
	}

	for (std::vector<VirtualServer>::iterator vs_it =
			 virtual_server_copy.begin();
		 vs_it != virtual_server_copy.end();) {
		if (ports_to_remove.count(vs_it->port) > 0 && !(vs_it->IP.empty()))
			vs_it = virtual_server_copy.erase(vs_it);
		else
			vs_it++;
	}

	std::set<Listen> trimmedVirtualServers;

	for (std::vector<VirtualServer>::const_iterator vs_it =
			 virtual_server_copy.begin();
		 vs_it != virtual_server_copy.end(); vs_it++) {
		Listen address(vs_it->IP, vs_it->port);
		trimmedVirtualServers.insert(address);
	}

	return trimmedVirtualServers;
}

// Sets up listening sockets
void Cluster::setupCluster(void) {
	createEpollInstance();	// Creates epoll instance

	std::set<Listen> trimmed_servers = trimVirtualServers();

	for (std::set<Listen>::const_iterator it = trimmed_servers.begin();
		 it != trimmed_servers.end(); it++) {
		Listen address(it->IP, it->port);
		// Create, bind and set up a new socket
		int sock_fd = createAndBindSocket(it->IP, it->port);

		// Starts listening to socket
		startListening(sock_fd);

		// Adds socket to epoll for monitoring
		addSocketsToEpoll(sock_fd);

		// Adds listening socket to _listening_socket vector
		_listening_sockets.push_back(sock_fd);
	}
}

// Sets an infinite loop to listen to incoming connections
void Cluster::run(void) {
	std::vector<struct epoll_event> events(MAX_CONNECTIONS);

	while (true) {
		int n = epoll_wait(_epoll_fd, &events[0], MAX_CONNECTIONS, -1);
		if (n == -1) throw ClusterSetupError("epoll_wait");

		for (int i = 0; i < n; ++i) {
			int fd = events[i].data.fd;

			if (isListeningSocket(fd))
				handleNewConnection(fd);
			else
				handleClientRequest(fd);
		}
	}
}

// Checks if fd corresponds to listening socket
bool Cluster::isListeningSocket(int fd) {
	return std::find(_listening_sockets.begin(), _listening_sockets.end(),
					 fd) != _listening_sockets.end();
}

// Handles a new connection
void Cluster::handleNewConnection(int listening_fd) {
	int client_fd = accept(listening_fd, NULL, NULL);
	if (client_fd == -1) throw ClusterSetupError("accept");

	setSocketToNonBlocking(client_fd);

	struct epoll_event client_event;
	client_event.events = EPOLLIN | EPOLLOUT | EPOLLET;
	client_event.data.fd = client_fd;

	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) == -1)
		throw ClusterRunError("epoll_ctl");
}

// Handles a client request
void Cluster::handleClientRequest(int connection_fd) {
	char buffer_request[BUFFER_SIZE] = {};
	ssize_t count =
		recv(connection_fd, buffer_request, sizeof(buffer_request), 0);

	if (count == -1) {
		if (errno != EAGAIN) {
			closeAndRemoveSocket(connection_fd, _epoll_fd);
			throw ClusterRunError("read failed");
		}
		return;	 // No data to read, just return
	}

	if (count == 0) {
		// Connection closed
		closeAndRemoveSocket(connection_fd, _epoll_fd);
		return;
	}

	std::string request(buffer_request, count);
	processRequest(connection_fd, request);
}

// Handles a client request
void Cluster::processRequest(int client_fd, const std::string& buffer_request) {
	HTTP_Request request;
	unsigned short error_status = HTTP_Request_Parser::parse_HTTP_request(
		buffer_request, request, client_fd, _epoll_fd);

	std::string buffer_response = getResponse(request, error_status, client_fd);

	ssize_t sent =
		send(client_fd, buffer_response.c_str(), buffer_response.size(), 0);

	if (sent == -1 && errno != EAGAIN) {
		closeAndRemoveSocket(client_fd, _epoll_fd);
		throw ClusterRunError("send failed");
	}

	closeAndRemoveSocket(client_fd, _epoll_fd);
}

// Sets sockets to non-blocking-mode
void Cluster::setSocketToNonBlocking(int socket_fd) {
	int flags = fcntl(socket_fd, F_GETFL, 0);
	if (flags == -1 || fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) == -1)
		throw ClusterSetupError("fcntl");
}

// Closes socket and removes it from epoll
void Cluster::closeAndRemoveSocket(int connecting_socket_fd, int epoll_fd) {
	close(connecting_socket_fd);
	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, connecting_socket_fd, NULL);
}

// Gets response from server
const std::string Cluster::getResponse(const HTTP_Request& request,
									   unsigned short& error_status,
									   int client_fd) {
	AResponse* response_check;

	const Server* server = getContext(client_fd, request);

	// TODO: Remove the following line! Just here for testing!
	error_status = OK;

	if (error_status != OK)
		response_check =
			new RequestErrorResponse(*server, request, error_status);
	else {
		switch (static_cast<int>(request.method)) {
			case (GET):
				response_check = new GetResponse(*server, request);
				break;
			case (POST):
				response_check =
					new PostResponse(*server, request, client_fd, _epoll_fd);
				break;
			case (DELETE):
				response_check = new DeleteResponse(*server, request);
				break;
		}
	}
	std::string response = response_check->generateResponse();
	delete response_check;

	return response;
}

const Listen Cluster::getListenFromClient(int client_fd) {
	struct sockaddr addr;
	socklen_t addr_len = sizeof(addr);
	struct sockaddr_in* addr_in;
	Listen address;

	getsockname(client_fd, &addr, &addr_len);
	addr_in = reinterpret_cast<struct sockaddr_in*>(&addr);
	address.port = numberToString(ntohs(addr_in->sin_port));
	address.IP = inet_ntoa(addr_in->sin_addr);

	return address;
}

const std::string Cluster::getHostNameFromRequest(const HTTP_Request& request) {
	std::multimap<std::string, std::string>::const_iterator host_name =
		request.header_fields.find("host");

	if (host_name != request.header_fields.end())
		return (host_name->second);
	else
		return "";
}

// Gets correct context for interpretation
const Server* Cluster::getContext(int client_fd, const HTTP_Request& request) {
	const Listen address = getListenFromClient(client_fd);
	std::string host_name = getHostNameFromRequest(request);
	size_t colonPos = host_name.find_first_of(':');
	if (colonPos != std::string::npos)
		host_name = host_name.substr(0, colonPos);
	std::vector<const Server*> valid_servers;

	// Traverses through all the server contexts in search of addresses
	for (std::vector<const Server*>::const_iterator server_it =
			 _servers.begin();
		 server_it != _servers.end(); server_it++) {
		// Gets the vector of addresses for a given server context
		std::vector<Listen> network_addresses =
			(*server_it)->getNetworkAddress();

		// For each address, check if it matches the IP:Port on the request
		for (std::vector<Listen>::const_iterator listen_it =
				 network_addresses.begin();
			 listen_it != network_addresses.end(); listen_it++)
			if (*listen_it == address) valid_servers.push_back(*server_it);
	}

	// If no addresses were found, check if it matches just the Port
	if (valid_servers.empty()) {
		for (std::vector<const Server*>::const_iterator server_it =
				 _servers.begin();
			 server_it != _servers.end(); server_it++) {
			std::vector<Listen> network_addresses =
				(*server_it)->getNetworkAddress();
			for (std::vector<Listen>::const_iterator listen_it =
					 network_addresses.begin();
				 listen_it != network_addresses.end(); listen_it++)
				if (listen_it->port == address.port)
					valid_servers.push_back(*server_it);
		}
	}

	// If multiple servers were found, return the first one with matching
	// server_names
	if (valid_servers.size() > 1) {
		for (std::vector<const Server*>::const_iterator valid_servers_it =
				 valid_servers.begin();
			 valid_servers_it != valid_servers.end(); valid_servers_it++) {
			std::vector<std::string> server_names =
				(*valid_servers_it)->getServerName();
			if (std::find(server_names.begin(), server_names.end(),
						  host_name) != server_names.end())
				return &(**valid_servers_it);
		}
	}

	// If no server names were found, just return the first one
	//  TODO: Check in case of URIs??
	//  TODO: Check in case of defaults?? (perhaps switch them to the first
	//  position in that case)
	return valid_servers.front();
}

// Getters for private member data
const std::vector<const Server*>& Cluster::getServerList() const {
	return _servers;
}
const std::vector<int>& Cluster::getListeningSockets() const {
	return _listening_sockets;
}

int Cluster::getEpollFd() const { return _epoll_fd; }

// Outputs Cluster data (epoll_fd, plus all the servers)
std::ostream& operator<<(std::ostream& outstream, const Cluster& cluster) {
	const std::vector<const Server*> server_list = cluster.getServerList();

	outstream << "The Cluster has an epoll_fd of [" << cluster.getEpollFd()
			  << "] and " << server_list.size() << " servers:" << std::endl;

	for (size_t i = 0; i < server_list.size(); i++)
		outstream << cluster[i] << std::endl;

	outstream << std::endl;

	return (outstream);
}
