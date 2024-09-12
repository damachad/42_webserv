/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cluster.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mde-sa-- <mde-sa--@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/19 14:44:25 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/08/21 16:13:50 by mde-sa--         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLUSTER_HPP
#define CLUSTER_HPP

#include "HTTPRequestParser.hpp"
#include "Server.hpp"
#include "ServerContext.hpp"
#include "Webserv.hpp"

#define BUFFER_SIZE 8096

class Cluster {
   public:
	// Constructor; create a vector of servers from provided context vector
	Cluster(const std::vector<ServerContext>& servers);

	// Destructor, closes _epoll_fd if it opened
	~Cluster();

	// Accesses ith server of _server array when asking Cluster[i]
	const Server& operator[](unsigned int index) const;

	// Sets up _epoll_fd, fills _listening_fd_map and instructs setup_server()
	void setup_cluster(void);

	// Adds socket to epoll so they can be monitored
	void add_sockets_to_epoll(const Server& server);

	// Sets an infinite loop to listen to incoming connections
	void run();

	// Getters for private member data
	const std::vector<Server>& get_server_list() const;
	const std::vector<int>& get_listening_sockets() const;
	const std::map<int, int>& get_listening_fd_map() const;
	const std::map<int, int>& get_connection_fd_map() const;
	int get_epoll_fd() const;

	// Returns respective server from each fd
	Server& get_server_from_listening_fd(int listening_fd);
	Server& get_server_from_connection_fd(int connection_fd);

   private:
	// Sets sockets to non-blocking-mode
	static void set_socket_to_non_blocking(int socket_fd);

	// Closes socket and removes it from epoll
	static void close_and_remove_socket(int connecting_socket_fd, int epoll_fd);

	// Placeholder function to get response
	const std::string get_response(const std::string& buffer_request,
								   const Server& server);

	// Vector of available servers
	std::vector<Server> _servers;

	// Vector of all listening fds;
	std::vector<int> _listening_sockets;

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
