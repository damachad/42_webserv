#ifndef CLUSTER_HPP
#define CLUSTER_HPP

#include "Webserv.hpp"

class Server;

class Cluster {
   public:
	// Constructor; create a vector of servers from provided context vector
	Cluster(std::vector<struct Context> servers);

	// Destructor, closes _epoll_fd if it opened
	~Cluster();

	// Accesses ith server of _server array when asking Cluster[i]
	const Server& operator[](unsigned int index) const;

	// Sets up _epoll_fd, fills _listening_fd_map and instructs setup_server()
	void setup_cluster(void);

	// Getters for private member data
	const std::vector<Server> get_server_list() const;
	const std::map<int, int> get_listening_fd_map() const;
	const std::map<int, int> get_connection_fd_map() const;
	int get_epoll_fd() const;

	// Returns respective server from each fd
	Server& get_server_from_listening_fd(int listening_fd);
	Server& get_server_from_connection_fd(int connection_fd);

   private:
	// Vector of available servers
	std::vector<Server> _servers;

	// Map that relates listening_fd to respective index on _server
	std::map<int, int> _listening_fd_map;

	// Map that relates _connection_fd to respective index on _server
	std::map<int, int> _connection_fd_map;

	// _epoll_fd for epoll()
	int _epoll_fd;

	// Constructors not to be used
	Cluster();
	Cluster(const Cluster& copy);
	const Cluster& operator=(const Cluster& copy);
};

std::ostream& operator<<(std::ostream& outstream, const Cluster& cluster);

#endif
