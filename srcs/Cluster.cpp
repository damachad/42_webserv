#include "Cluster.hpp"

#include "Exceptions.hpp"
#include "Helpers.hpp"
#include "Server.hpp"

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
	// TODO: Call function that sets up epoll_fd

	for (size_t i = 0; i < _servers.size(); i++) {
		Server server = _servers[i];

		// Sets up every port on each server
		server.setup_server();

		// Adds ports to _listening_fd_map for easier access
		const std::vector<int> socks_listing = server.get_listening_sockets();
		for (size_t j = 0; j < socks_listing.size(); j++)
			_listening_fd_map[socks_listing[j]] = i;
	}
}

// Getters for private member data
const std::vector<Server> Cluster::get_server_list() const { return _servers; }
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
