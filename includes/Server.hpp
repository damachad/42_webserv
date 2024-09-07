/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mde-sa-- <mde-sa--@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/19 14:44:29 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/08/19 14:57:44 by mde-sa--         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "Webserv.hpp"

struct Listen;

class Server {
   public:
	// Constructor, creates server from configuration file
	Server(const ServerContext& configuration);

	// Destructor, closes all listening sockets
	~Server();

	// Sets up Server and adds sockets to _listening_sockets
	void setup_server(void);

	// Getters for private member data
	const std::vector<std::string>& get_server_names() const;
	const std::vector<Listen>& get_network_addresses() const;
	const std::vector<int>& get_listening_sockets() const;

   private:
	// Server's names in config
	const std::vector<std::string> _server_names;

	// Server's network addresses (IP and Port)
	const std::vector<Listen> _network_addresses;

	// Server's listening fds (opened by socket)
	std::vector<int> _listening_sockets;

	// Server's sockaddr vector
	std::vector<struct sockaddr_in> _sockaddr_vector;

	// Constructors (not to be used)
	Server();
	const Server& operator=(const Server& copy);
	// Server(const Server& copy); NOTE: Needed for Cluster[i] access!
};

// Outputs Server's Hostname and Ports
std::ostream& operator<<(std::ostream& outstream, const Server& server);

#endif
