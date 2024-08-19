/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mde-sa-- <mde-sa--@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/19 14:38:07 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/08/19 15:30:40 by mde-sa--         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

// Constructor, creates server from configuration file
Server::Server(const struct Context& configuration)
	: _server_names(configuration.serverName),
	  _network_addresses(configuration.network_address),
	  _listening_sockets(),
	  _sockaddr_vector() {
	//	std::cout << "Server Constructor called" << std::endl;

	std::cout << configuration << std::endl;
}

// Destructor, closes all listening sockets
Server::~Server() {
	//	std::cout << "Server Destructor called" << std::endl;
	for (std::vector<int>::iterator it = _listening_sockets.begin();
		 it != _listening_sockets.end(); it++)
		close(*it);
}

// Sets up Server and adds sockets to _listening_sockets
void Server::setup_server(void) {
	const std::vector<Listen> network_addresses = get_network_addresses();

	for (std::vector<Listen>::const_iterator it = network_addresses.begin();
		 it != network_addresses.end(); it++) {
		// Create a socket (IPv4, TCP)
		int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (sock_fd == -1) throw SocketSetupError("socket");

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
		sockaddr.sin_port = htons(
			string_to_int(it->port));  // Converts number to network byte order

		// Binds name to socket
		if (bind(sock_fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
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

// Getters for private member data
const std::vector<std::string>& Server::get_server_names(void) const {
	return _server_names;
}
const std::vector<Listen>& Server::get_network_addresses(void) const {
	return _network_addresses;
}
const std::vector<int>& Server::get_listening_sockets(void) const {
	return _listening_sockets;
}

// Outputs Server's Hostname and Ports
std::ostream& operator<<(std::ostream& outstream, const Server& server) {
	std::vector<std::string> server_names = server.get_server_names();

	outstream << "Server [" << server_names[0]
			  << "] is listening to addresses:";

	const std::vector<Listen> network_addresses =
		server.get_network_addresses();
	for (std::vector<Listen>::const_iterator it = network_addresses.begin();
		 it != network_addresses.end(); it++)
		outstream << "[" << (*it).IP << ":" << (*it).port << "]" << std::endl;

	outstream << std::endl;

	return (outstream);
}
