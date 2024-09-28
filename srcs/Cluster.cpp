/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cluster.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/19 14:44:19 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/09/24 10:33:42 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cluster.hpp"

#include <utility>

#include "AResponse.hpp"
#include "DeleteResponse.hpp"
#include "GetResponse.hpp"
#include "HTTPRequestParser.hpp"
#include "PostResponse.hpp"
#include "RequestErrorResponse.hpp"

// Constructor
// Create a vector of servers from provided context vector
Cluster::Cluster(const std::vector<Server>& servers)
	: _servers(), _listening_fd_map(), _connection_fd_map(), _epoll_fd(-1) {
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

void Cluster::create_epoll_instance(void) {
	_epoll_fd = epoll_create(1);  // Int is ignored in newer implementations
	if (_epoll_fd == -1) throw ClusterSetupError("epoll_create");
}

int Cluster::create_and_bind_socket(const std::string& IP,
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
void Cluster::start_listening(int sock_fd) {
	if (listen(sock_fd, SOMAXCONN) < 0) {
		close(sock_fd);
		throw SocketSetupError("listen");
	}
}

// Adds sockets to epoll so they can be monitored
void Cluster::add_sockets_to_epoll(int sock_fd) {
	epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = sock_fd;

	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, sock_fd, &event) == -1)
		throw ClusterSetupError("epoll_ctl");
}

bool hasIP(const VirtualServer& vs) { return !vs.IP.empty(); }

void Cluster::reorderVirtualServers() {
	std::stable_partition(_virtual_servers.begin(), _virtual_servers.end(),
						  hasIP);
}

// Sets up listening sockets
void Cluster::setup_cluster(void) {
	create_epoll_instance();  // Creates epoll instance

	// Map that takes note of used fds
	std::map<Listen, int> used_fds;

	reorderVirtualServers();

	for (std::vector<VirtualServer>::const_iterator it =
			 _virtual_servers.begin();
		 it != _virtual_servers.end(); it++) {
		Listen address(it->IP, it->port);
		if (used_fds.find(address) !=
			used_fds.end())	 // If fd has already been used, add it to map
		{
			int sock_fd = used_fds[address];
			_listening_fd_map[sock_fd].push_back(it->server);
		} else {
			// Create, bind and set up a new socket
			int sock_fd = create_and_bind_socket(it->IP, it->port);

			// Starts listening to socket
			start_listening(sock_fd);

			// Adds socket to epoll for monitoring
			add_sockets_to_epoll(sock_fd);

			// Track IP:Port <-> socket combination
			used_fds[address] = sock_fd;

			// Track listening_port -> Server
			_listening_fd_map.insert(std::make_pair(
				sock_fd, std::vector<const Server*>(
							 1, it->server)));	// Add sock_fd and server to
												// listening fd map
		}
	}
}

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

bool Cluster::isListeningSocket(int fd) {
	return std::find(_listening_sockets.begin(), _listening_sockets.end(),
					 fd) != _listening_sockets.end();
}

void Cluster::handleNewConnection(int listening_fd) {
	int client_fd = accept(listening_fd, NULL, NULL);
	if (client_fd == -1) throw ClusterSetupError("accept");

	set_socket_to_non_blocking(client_fd);
	_connection_fd_map[client_fd] =
		_listening_fd_map.find(listening_fd)->second;

	struct epoll_event client_event;
	client_event.events = EPOLLIN | EPOLLOUT | EPOLLET;
	client_event.data.fd = client_fd;

	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event) == -1)
		throw ClusterRunError("epoll_ctl");
}

void Cluster::handleClientRequest(int client_fd) {
	char buffer_request[BUFFER_SIZE] = {};
	ssize_t count = read(client_fd, buffer_request, sizeof(buffer_request));

	if (count == -1) {
		if (errno != EAGAIN) {
			close_and_remove_socket(client_fd, _epoll_fd);
			throw ClusterRunError("read failed");
		}
		return;	 // No data to read, just return
	}

	if (count == 0) {
		// Connection closed
		close_and_remove_socket(client_fd, _epoll_fd);
		return;
	}

	std::string request(buffer_request, BUFFER_SIZE);
	processRequest(client_fd, request, count);
}

void Cluster::processRequest(int client_fd, const std::string& buffer_request,
							 ssize_t count) {
	// Assume we have a buffer for the client
	_client_buffer_map[client_fd].append(buffer_request.c_str(), count);

	HTTP_Request request;
	unsigned short error_status = HTTP_Request_Parser::parse_HTTP_request(
		_client_buffer_map[client_fd], request);

	// if (error_status == CONTINUE &&
	// 	(request.method == GET || request.method == DELETE))
	// 	;  // Send continue message

	/*if (error_status != CONTINUE) {
		std::string buffer_response = get_response(
			request, error_status, *_servers[_connection_fd_map[client_fd]]);

		ssize_t sent =
			send(client_fd, buffer_response.c_str(), buffer_response.size(), 0);
		if (sent == -1 && errno != EAGAIN) {
			close_and_remove_socket(client_fd, _epoll_fd);
			throw ClusterRunError("send failed");
		}
		close_and_remove_socket(client_fd, _epoll_fd);

		// Clear buffer after processing the request
		_client_buffer_map[client_fd].clear();
	} else if (error_status == CONTINUE && request.method == POST) {
		// Wait for more data;
	}*/
	(void)error_status;
}

// Sets sockets to non-blocking-mode
void Cluster::set_socket_to_non_blocking(int socket_fd) {
	int flags = fcntl(socket_fd, F_GETFL, 0);
	if (flags == -1 || fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) == -1)
		throw ClusterSetupError("fcntl");
}

// Closes socket and removes it from epoll
void Cluster::close_and_remove_socket(int connecting_socket_fd, int epoll_fd) {
	close(connecting_socket_fd);
	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, connecting_socket_fd, NULL);
}

// Placeholder function to get response
const std::string Cluster::get_response(const HTTP_Request& request,
										unsigned short& error_status,
										const Server& server) {
	std::vector<std::string> server_name = server.getServerName();

	std::string response;

	AResponse* response_check;

	if (error_status != OK)
		response_check =
			new RequestErrorResponse(server, request, error_status);
	else {
		switch (static_cast<int>(request.method)) {
			case (GET):
				response_check = new GetResponse(server, request);
				break;
			case (POST):
				response_check = new PostResponse(server, request);
				break;
			case (DELETE):
				response_check = new DeleteResponse(server, request);
				break;
		}
	}

	return response_check->generateResponse();
}

// Getters for private member data
const std::vector<const Server*>& Cluster::get_server_list() const {
	return _servers;
}
const std::vector<int>& Cluster::get_listening_sockets() const {
	return _listening_sockets;
}
const std::map<int, std::vector<const Server*> >&
Cluster::get_listening_fd_map() const {
	return _listening_fd_map;
}
const std::map<int, std::vector<const Server*> >&
Cluster::get_connection_fd_map() const {
	return _connection_fd_map;
}
int Cluster::get_epoll_fd() const { return _epoll_fd; }

// Returns respective server from each fd
const std::vector<const Server*>& Cluster::get_server_from_listening_fd(
	int listening_fd) {
	if (_listening_fd_map.find(listening_fd) == _listening_fd_map.end())
		throw ValueNotFoundError(numberToString<int>(listening_fd));

	return (_listening_fd_map.find(listening_fd)->second);
}

const std::vector<const Server*>& Cluster::get_server_from_connection_fd(
	int connection_fd) {
	if (_connection_fd_map.find(connection_fd) == _connection_fd_map.end()) {
		throw ValueNotFoundError(numberToString<int>(connection_fd));
	}
	return _connection_fd_map.find(connection_fd)->second;
}

// Outputs Cluster data (epoll_fd, plus all the servers)
std::ostream& operator<<(std::ostream& outstream, const Cluster& cluster) {
	const std::vector<const Server*> server_list = cluster.get_server_list();

	outstream << "The Cluster has an epoll_fd of [" << cluster.get_epoll_fd()
			  << "] and " << server_list.size() << " servers:" << std::endl;

	for (size_t i = 0; i < server_list.size(); i++)
		outstream << cluster[i] << std::endl;

	outstream << std::endl;

	return (outstream);
}
