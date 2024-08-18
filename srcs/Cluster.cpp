#include "Cluster.hpp"

#include <cmath>

#include "Exceptions.hpp"
#include "Helpers.hpp"
#include "Server.hpp"
#include "Webserv.hpp"

// Constructor
// Create a vector of servers from provided context vector
Cluster::Cluster(std::vector<struct Context> servers)
	: _servers(), _listening_fd_map(), _connection_fd_map(), _epoll_fd(-1) {
	for (std::vector<struct Context>::iterator it = servers.begin();
		 it != servers.end(); it++)
		_servers.push_back(Server(*it));
}

// Destructor
// Closes _epoll_fd if it was open
Cluster::~Cluster() {
	if (_epoll_fd >= 0) close(_epoll_fd);
}

// Accesses ith server of _server array when asking Cluster[i]
const Server& Cluster::operator[](unsigned int index) const {
	if (index >= _servers.size()) throw OutOfBoundsError(int_to_string(index));

	return _servers[index];
}

// Sets up _epoll_fd, fills _listening_fd_map and instructs setup_server()
void Cluster::setup_cluster(void) {
	_epoll_fd = epoll_create(1);
	if (_epoll_fd == -1) throw ClusterSetupError("epoll_create");

	/*for (size_t i = 0; i < _servers.size(); i++) {
		Server& server = _servers[i];

		// Sets up every port on each server
		server.setup_server();

		// Adds sockets to epoll instance
		add_sockets_to_epoll(server);

		// Adds ports to _listening_sockets and to _listening_fd_map
		const std::vector<int> socks_listing = server.get_listening_sockets();
		for (std::vector<int>::const_iterator it = socks_listing.begin();
			 it < socks_listing.end(); it++) {
			_listening_sockets.push_back(*it);
			_listening_fd_map[*it] = i;	 // NOTE: POSSIBLY UNNEDED?
		}
	}*/

	int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd == -1) throw SocketSetupError("socket");

	// Listen to connections on socket (port given by *it)
	struct sockaddr_in sockaddr;
	std::memset(&sockaddr, 0, sizeof(sockaddr));  // Clears the struct
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	sockaddr.sin_port = htons(1025);  // Converts number to network byte order

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

	epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = sock_fd;
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, sock_fd, &event) == -1)
		throw ClusterSetupError("epoll_ctl");

	std::vector<struct epoll_event> events(MAX_CONNECTIONS);

	while (true) {
		int n = epoll_wait(_epoll_fd, &events[0], MAX_CONNECTIONS, -1);
		if (n == -1) throw ClusterSetupError("epoll_wait");
	}
}

// Adds sockets to epoll so they can be monitored
void Cluster::add_sockets_to_epoll(const Server& server) {
	std::vector<int> socket_list = server.get_listening_sockets();

	for (std::vector<int>::const_iterator it = socket_list.begin();
		 it != socket_list.end(); it++) {
		int listening_socket = *it;
		epoll_event event;
		event.events = EPOLLIN;
		event.data.fd = listening_socket;

		if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, listening_socket, &event) == -1)
			throw ClusterSetupError("epoll_ctl");
	}
}

void Cluster::run(void) {
	std::vector<struct epoll_event> events(MAX_CONNECTIONS);

	while (true) {
		int n = epoll_wait(_epoll_fd, &events[0], MAX_CONNECTIONS, -1);
		if (n == -1) throw ClusterSetupError("epoll_wait");

		for (int i = 0; i < n; ++i) {
			if (std::find(_listening_sockets.begin(), _listening_sockets.end(),
						  events[i].data.fd) != _listening_sockets.end()) {
				// New connection on listening socket
				int client_fd = accept(events[i].data.fd, NULL, NULL);
				if (client_fd == -1) throw ClusterSetupError("accept");

				struct epoll_event client_event;
				client_event.events = EPOLLIN | EPOLLOUT | EPOLLET;
				client_event.data.fd = client_fd;
				if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd,
							  &client_event) == -1)
					throw ClusterSetupError("epoll_ctl");
			} else {
				// Handle I/O on connected socket
				char buffer[1024];
				ssize_t count = read(events[i].data.fd, buffer, sizeof(buffer));
				if (count == -1) {
					if (errno != EAGAIN) {
						perror("read failed");
						close(events[i].data.fd);
						epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, events[i].data.fd,
								  NULL);
					}
				} else if (count == 0) {
					// Connection closed
					close(events[i].data.fd);
					epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, events[i].data.fd,
							  NULL);
				} else {
					// Echo the data back (for example purposes)
					ssize_t sent = send(events[i].data.fd, buffer, count, 0);
					if (sent == -1 && errno != EAGAIN) {
						perror("send failed");
						close(events[i].data.fd);
						epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, events[i].data.fd,
								  NULL);
					}
				}
			}
		}
	}
}

// Getters for private member data
const std::vector<Server> Cluster::get_server_list() const { return _servers; }
const std::vector<int> Cluster::get_listening_sockets() const {
	return _listening_sockets;
}
const std::map<int, int> Cluster::get_listening_fd_map() const {
	return _listening_fd_map;
}
const std::map<int, int> Cluster::get_connection_fd_map() const {
	return _connection_fd_map;
}
int Cluster::get_epoll_fd() const { return _epoll_fd; }

// Returns respective server from each fd
Server& Cluster::get_server_from_listening_fd(int listening_fd) {
	if (_listening_fd_map.find(listening_fd) == _listening_fd_map.end())
		throw ValueNotFoundError(int_to_string(listening_fd));

	return _servers[_listening_fd_map[listening_fd]];
}

Server& Cluster::get_server_from_connection_fd(int connection_fd) {
	if (_connection_fd_map.find(connection_fd) == _connection_fd_map.end()) {
		throw ValueNotFoundError(int_to_string(connection_fd));
	}
	return _servers[_connection_fd_map[connection_fd]];
}

// Outputs Cluster data (epoll_fd, plus all the servers)
std::ostream& operator<<(std::ostream& outstream, const Cluster& cluster) {
	std::vector<Server> server_list = cluster.get_server_list();

	outstream << "The Cluster has an epoll_fd of [" << cluster.get_epoll_fd()
			  << "] and " << server_list.size() << " servers:" << std::endl;

	for (size_t i = 0; i < server_list.size(); i++)
		outstream << cluster[i] << std::endl;

	outstream << std::endl;

	return (outstream);
}
