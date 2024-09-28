/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cluster.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mde-sa-- <mde-sa--@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/19 14:44:25 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/09/14 09:57:07 by mde-sa--         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLUSTER_HPP
#define CLUSTER_HPP

#include "Server.hpp"

#define BUFFER_SIZE 8096
#define HTTP_REQUEST_INCOMPLETE 2000  // TODO: CHANGE

class Server;
struct HTTP_Request;

struct VirtualServer {
	std::string IP;
	std::string port;
	std::string server_name;
	const Server* server;

	// Comparison operator for std::set
	bool operator<(const VirtualServer& other) const {
		// Compare based on IP, port, and server_name
		if (IP != other.IP) return IP < other.IP;
		if (port != other.port) return port < other.port;
		return server_name < other.server_name;
	}
};

class Cluster {
   public:
	// Constructor; create a vector of servers from provided context vector
	Cluster(const std::vector<Server>& servers);

	// Destructor, closes _epoll_fd if it opened
	~Cluster();

	// Accesses ith server of _server array when asking Cluster[i]
	const Server& operator[](unsigned int index) const;

	// Checks there are repeated virtual addresses with the same contents
	bool hasDuplicateVirtualServers() const;

	// Sets up _epoll_fd, fills _listening_fd_map and instructs setup_server()
	void create_epoll_instance(void);
	std::set<Listen> trimVirtualServers();
	void setup_cluster(void);
	int create_and_bind_socket(const std::string& IP, const std::string& port);
	void start_listening(int sock_fd);
	void add_sockets_to_epoll(int sock_fd);

	// Sets an infinite loop to listen to incoming connections
	void run();
	bool isListeningSocket(int fd);
	void handleNewConnection(int listening_fd);
	void handleClientRequest(int client_fd);
	void processRequest(int client_fd, const std::string& buffer_request,
						ssize_t count);

	// Getters for private member data
	const std::vector<const Server*>& get_server_list() const;
	const std::vector<VirtualServer> get_virtual_server_list() const;
	const std::vector<int>& get_listening_sockets() const;
	const std::map<int, std::vector<const Server*> >& get_listening_fd_map()
		const;
	const std::map<int, std::vector<const Server*> >& get_connection_fd_map()
		const;
	int get_epoll_fd() const;

	// Returns respective server from each fd
	const std::vector<const Server*>& get_server_from_listening_fd(
		int listening_fd);
	const std::vector<const Server*>& get_server_from_connection_fd(
		int connection_fd);

	// Reorders _virtual_servers so it contains no-IP elements last
	void reorderVirtualServers();

   private:
	// Sets sockets to non-blocking-mode
	static void set_socket_to_non_blocking(int socket_fd);

	// Closes socket and removes it from epoll
	static void close_and_remove_socket(int connecting_socket_fd, int epoll_fd);

	// Placeholder function to get response
	const std::string get_response(const HTTP_Request& request,
								   unsigned short& error_status,
								   const Server& server);

	// Vector of available servers
	std::vector<const Server*> _servers;

	// Vector of all virtual servers
	std::vector<VirtualServer> _virtual_servers;

	// Vector of all listening fds
	std::vector<int> _listening_sockets;

	// Buffer map for all clients
	std::map<int, std::string> _client_buffer_map;

	// Map that relates listening_fd to respective server pointer
	std::map<int, std::vector<const Server*> > _listening_fd_map;

	// Map that relates _connection_fd to respective index on _server
	std::map<int, std::vector<const Server*> > _connection_fd_map;

	// _epoll_fd for epoll()
	int _epoll_fd;

	// Constructors not to be used
	Cluster();
	Cluster(const Cluster& copy);
	const Cluster& operator=(const Cluster& copy);
};

std::ostream& operator<<(std::ostream& outstream, const Cluster& cluster);

#endif
