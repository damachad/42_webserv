#include "Exceptions.hpp"
#include "Webserv.hpp"

// Constructor, creates server from configuration file
Server::Server(const struct Context& configuration)
	: _hostname(configuration.serverName),
	  _ports(configuration.ports),
	  _listening_sockets(),
	  _sockaddr_vector() {
	std::cout << "Created a server!" << std::endl;

	std::cout << configuration << std::endl;
}

// Destructor, closes all listening sockets
Server::~Server() {
	for (std::vector<int>::iterator it = _listening_sockets.begin();
		 it != _listening_sockets.end(); it++)
		close(*it);
}

// Sets up Server and adds sockets to _listening_sockets
void Server::setup_server(void) {
	const std::vector<uint16_t> ports_listing = get_ports();

	for (std::vector<uint16_t>::const_iterator it = ports_listing.begin();
		 it != ports_listing.end(); it++) {
		// Create a socket (IPv4, TCP)
		int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (sock_fd == -1) throw SocketSetupError("socket");

		// Listen to connections on socket (port given by *it)
		struct sockaddr_in sockaddr;
		std::memset(&sockaddr, 0, sizeof(sockaddr));  // Clears the struct
		sockaddr.sin_family = AF_INET;
		sockaddr.sin_addr.s_addr =
			inet_addr("127.0.0.1");	 // NOTE: INADDR_ANY? htonl host? Get a
									 // _host var?
		sockaddr.sin_port =
			htons(*it);	 // Converts number to network byte order

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

		// Adds sock_fd and sockaddr to the server object
		_listening_sockets.push_back(sock_fd);
		_sockaddr_vector.push_back(sockaddr);
	}
}

// Getters for private member data
const std::string Server::get_hostname(void) const { return _hostname; }
const std::vector<uint16_t> Server::get_ports(void) const { return _ports; }
const std::vector<int> Server::get_listening_sockets(void) const {
	return _listening_sockets;
}

// Outputs Server's Hostname and Ports
std::ostream& operator<<(std::ostream& outstream, const Server& server) {
	outstream << "Server [" << server.get_hostname()
			  << "] is listening to ports: [ ";

	const std::vector<uint16_t> server_ports = server.get_ports();
	for (std::vector<uint16_t>::const_iterator it = server_ports.begin();
		 it != server_ports.end(); it++)
		outstream << *it << " ";

	outstream << "]." << std::endl;

	return (outstream);
}
