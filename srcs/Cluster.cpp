/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cluster.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mde-sa-- <mde-sa--@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/19 14:44:19 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/08/21 16:14:51 by mde-sa--         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cluster.hpp"

#include "AResponse.hpp"
#include "PostResponse.hpp"

// Constructor
// Create a vector of servers from provided context vector
Cluster::Cluster(const std::vector<Server>& servers)
	: _servers(), _listening_fd_map(), _connection_fd_map(), _epoll_fd(-1) {
	//	std::cout << "Cluster Constructor called" << std::endl;
	for (std::vector<Server>::const_iterator it = servers.begin();
		 it != servers.end(); it++) {
		_servers.push_back(*it);
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

	return _servers[index];
}

// Sets up _epoll_fd, fills _listening_fd_map and instructs setup_server()
void Cluster::setup_cluster(void) {
	_epoll_fd = epoll_create(1);  // Int is ignored in newer implementations
	if (_epoll_fd == -1) throw ClusterSetupError("epoll_create");

	for (size_t i = 0; i < _servers.size(); i++) {
		Server& server = _servers[i];

		// Sets up every port on each server
		server.setup_server();

		// Adds sockets to epoll instance
		add_sockets_to_epoll(server);

		// Adds ports to _listening_sockets and to _listening_fd_map
		const std::vector<int> socks_listing = server.getListeningSockets();
		for (std::vector<int>::const_iterator it = socks_listing.begin();
			 it < socks_listing.end(); it++) {
			_listening_sockets.push_back(*it);
			_listening_fd_map[*it] = i;
		}
	}
	// std::cerr << "Here it goes?" << std::endl;
}

// Adds sockets to epoll so they can be monitored
void Cluster::add_sockets_to_epoll(const Server& server) {
	std::vector<int> socket_list = server.getListeningSockets();

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

				// Sets connection as non_blocking
				set_socket_to_non_blocking(client_fd);

				// Adds to client_fd -> server map
				_connection_fd_map[client_fd] =
					_listening_fd_map[events[i].data.fd];

				struct epoll_event client_event;
				client_event.events =
					EPOLLIN | EPOLLOUT | EPOLLET;  // Ready for input, output,
												   // and in Edge-Triggered-Mode
				client_event.data.fd = client_fd;
				if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd,
							  &client_event) == -1)
					throw ClusterRunError("epoll_ctl");
			} else {
				// Handle I/O on connected socket
				char buffer_request[BUFFER_SIZE];
				ssize_t count = read(events[i].data.fd, buffer_request,
									 sizeof(buffer_request));
				if (count == -1) {
					if (errno != EAGAIN) {
						close_and_remove_socket(events[i].data.fd, _epoll_fd);
						throw ClusterRunError("read failed");
					}
				} else if (count == 0)
					// Connection closed
					close_and_remove_socket(events[i].data.fd, _epoll_fd);
				// TODO: Remove from client_to_fd map ??
				else {
					HTTP_Request request =
						HTTP_Request_Parser::parse_HTTP_request(buffer_request);
					(void)request;
					//  Echo the data back (for example purposes)
					std::string buffer_response = get_response(
						request,
						_servers[_connection_fd_map[events[i].data.fd]]);
					ssize_t sent =
						send(events[i].data.fd, buffer_response.c_str(),
							 buffer_response.size(), 0);
					// ssize_t sent = send(events[i].data.fd, buffer, count, 0);
					if (sent == -1 && errno != EAGAIN) {
						close_and_remove_socket(events[i].data.fd, _epoll_fd);
						throw ClusterRunError("send failed");
					}
				}
			}
		}
	}
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
										const Server& server) {
	std::vector<std::string> server_name = server.getServerName();

	std::string HTTP_response;

	switch (request.method) {
		case (GET):
			// std::string = resultado da classe GetResponse
			// return std::string
			break;
		case (DELETE):
			// std::string = resultado da classe DeleteResponse
			// return std::string
			break;
		case (POST): {
			PostResponse ProcessResponse(server, request);
			HTTP_response = ProcessResponse.generateResponse();
		}
		case (UNKNOWN):
			// Provavelmente impossível de acontecer???
			break;
	}

	return HTTP_response;

	// NOTE: Isto irá desaparecer, fica só como placeholder para testes
	// Read html file to target, and get that to a body c-style STR
	std::string target = request.uri;
	if (target == "/") target = "/index.html";
	target = "resources" + target;
	std::ifstream filestream(target.c_str());
	std::stringstream buffer;
	buffer << filestream.rdbuf();
	std::string body = buffer.str();

	std::string body_len = numberToString<int>(static_cast<int>(body.size()));

	std::string response =
		"HTTP/1.1 200 OK\r\n"  // HTP 1.1 Header
		"Content-Type: text/html; charset=UTF-8\r\n"
		"Content-Length: " +
		body_len +	// Body Length
		"\r\n"
		"\r\n" +
		body;  // Body content;

	return response;
}

// Getters for private member data
const std::vector<Server>& Cluster::get_server_list() const { return _servers; }
const std::vector<int>& Cluster::get_listening_sockets() const {
	return _listening_sockets;
}
const std::map<int, int>& Cluster::get_listening_fd_map() const {
	return _listening_fd_map;
}
const std::map<int, int>& Cluster::get_connection_fd_map() const {
	return _connection_fd_map;
}
int Cluster::get_epoll_fd() const { return _epoll_fd; }

// Returns respective server from each fd
Server& Cluster::get_server_from_listening_fd(int listening_fd) {
	if (_listening_fd_map.find(listening_fd) == _listening_fd_map.end())
		throw ValueNotFoundError(numberToString<int>(listening_fd));

	return _servers[_listening_fd_map[listening_fd]];
}

Server& Cluster::get_server_from_connection_fd(int connection_fd) {
	if (_connection_fd_map.find(connection_fd) == _connection_fd_map.end()) {
		throw ValueNotFoundError(numberToString<int>(connection_fd));
	}
	return _servers[_connection_fd_map[connection_fd]];
}

// Outputs Cluster data (epoll_fd, plus all the servers)
std::ostream& operator<<(std::ostream& outstream, const Cluster& cluster) {
	const std::vector<Server>& server_list = cluster.get_server_list();

	outstream << "The Cluster has an epoll_fd of [" << cluster.get_epoll_fd()
			  << "] and " << server_list.size() << " servers:" << std::endl;

	for (size_t i = 0; i < server_list.size(); i++)
		outstream << cluster[i] << std::endl;

	outstream << std::endl;

	return (outstream);
}
