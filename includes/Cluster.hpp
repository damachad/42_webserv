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

	// Sets up listening sockets
	void setupCluster(void);

	// Sets an infinite loop to listen to incoming connections
	void run();

	// Getters for private member data
	const std::vector<const Server*>& getServerList() const;
	const std::vector<VirtualServer> getVirtualServerList() const;
	const std::vector<int>& getListeningSockets() const;
	const std::map<int, std::vector<const Server*> >& getListeningFdMap() const;
	const std::map<int, std::vector<const Server*> >& getConnectionFdMap()
		const;
	int getEpollFd() const;

   private:
	// Functions for setupCluster()
	//// Creates epoll instance
	void createEpollInstance(void);
	//// Gets list of sockets needed
	std::set<Listen> trimVirtualServers();
	//// Creates sockets and binds to them
	int createAndBindSocket(const std::string& IP, const std::string& port);
	//// Starts listening to incoming connections
	void startListening(int sock_fd);
	//// Adds sockets to epoll so they can be monitored
	void addSocketsToEpoll(int sock_fd);

	// Functions for run()
	//// Checks if fd correspondes to a listening socket
	bool isListeningSocket(int fd);
	//// Handles a new connection
	void handleNewConnection(int listening_fd);
	//// Sets sockets to non-blocking-mode
	static void setSocketToNonBlocking(int socket_fd);
	//// Handles a client request
	void handleClientRequest(int connection_fd);
	//// Processes the request
	void processRequest(int connection_fd, const std::string& buffer_request);

	//// Closes socket and removes it from epoll
	static void closeAndRemoveSocket(int connecting_socket_fd, int epoll_fd);

	//// Gets response from server
	const std::string getResponse(HTTP_Request& request,
								  unsigned short& error_status, int client_fd);
	// Gets correct context from client_fd
	const Server* getContext(int client_fd, const HTTP_Request& request);

	// Gets address from client_fd
	const Listen getListenFromClient(int client_fd);

	const std::string getHostNameFromRequest(const HTTP_Request& request);

	// Vector of available servers
	std::vector<const Server*> _servers;

	// Vector of all virtual servers
	std::vector<VirtualServer> _virtual_servers;

	// Vector of all listening fds
	std::vector<int> _listening_sockets;

	// _epoll_fd for epoll()
	int _epoll_fd;

	// Constructors not to be used
	Cluster();
	Cluster(const Cluster& copy);
	const Cluster& operator=(const Cluster& copy);
};

std::ostream& operator<<(std::ostream& outstream, const Cluster& cluster);

#endif
