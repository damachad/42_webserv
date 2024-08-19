/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mde-sa-- <mde-sa--@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/19 14:44:29 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/08/19 14:44:30 by mde-sa--         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "Webserv.hpp"

class Server {
   public:
	// Constructor, creates server from configuration file
	Server(const struct Context& configuration);

	// Destructor, closes all listening sockets
	~Server();

	// Sets up Server and adds sockets to _listening_sockets
	void setup_server(void);

	// Getters for private member data
	const std::vector<std::string> get_server_names() const;
	const std::vector<Listen> get_network_addresses() const;
	const std::vector<int> get_listening_sockets() const;

   private:
	// Server's hostname
	const std::vector<std::string> _server_names;

	// Server's network addresses
	const std::vector<Listen> _network_addresses;

	// Server's listening fds
	std::vector<int> _listening_sockets;

	// Server's sockaddr vector
	std::vector<struct sockaddr_in> _sockaddr_vector;

	// Constructors not to be used
	Server();
	const Server& operator=(const Server& copy);
	// Server(const Server& copy); NOTE: Needed for Cluster[i] access!
};

// Outputs Server's Hostname and Ports
std::ostream& operator<<(std::ostream& outstream, const Server& server);

#endif
