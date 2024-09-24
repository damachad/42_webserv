/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/01 11:53:15 by damachad          #+#    #+#             */
/*   Updated: 2024/09/18 16:08:23 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cluster.hpp"
#include "ConfigParser.hpp"
#include "Server.hpp"
#include "Webserv.hpp"

int main(int argc, char** argv) {
	// Get config
	if (argc > 2) {
		std::cout << "Usage: ./webserv [configuration file]\n";
		return (1);
	}
	ConfigParser parser("conf/default2.conf");
	if (argc == 2)
		parser = ConfigParser(argv[1]);

	std::vector<Server>
		servers;  // Defined outside the try block so it can be used later

	try {
		parser.loadConfigs();
		servers = parser.getServers();
		// for (std::vector<Server>::const_iterator it = servers.begin();
		// 	 it != servers.end(); ++it)
		// 	std::cout << (*it) << "\n";
		// std::string route = "/test";
		// unsigned int serverNum = 0;
		//  parser.printLocationValues(serverNum, route);
	} catch (std::exception& e) {
		std::cerr << e.what();
		return 1;
	}

	// Initializes the Server Cluster
	Cluster server_cluster(servers);

	// Attempts to Setup the Cluster
	try {
		server_cluster.setup_cluster();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}

	std::cout << server_cluster;

	server_cluster.run();

	/*
		// Grab a connection from the queue
		socklen_t addrlen = sizeof(sockaddr);
		int connection = accept(sockfd, (struct sockaddr*)&sockaddr, &addrlen);
		if (connection < 0) {
			std::cout << "Failed to grab connection. errno: " << errno <<
	   std::endl; exit(EXIT_FAILURE);
		}

		// Read from the connection
		char buffer[100];
		ssize_t bytesRead = read(connection, buffer, sizeof(buffer) - 1);
		if (bytesRead < 0) {
			std::cout << "Failed to read from connection. errno: " << errno
					  << std::endl;
			close(connection);
			close(sockfd);
			exit(EXIT_FAILURE);
		}
		buffer[bytesRead] = '\0';  // Null-terminate the buffer
		std::cout << "The message was: " << buffer << std::endl;

		// Send a message to the connection
		std::string response = "Good talking to you\n";
		ssize_t bytesSent = send(connection, response.c_str(), response.size(),
	   0); if (bytesSent < 0) { std::cout << "Failed to send response. errno: "
	   << errno << std::endl; close(connection); close(sockfd);
			exit(EXIT_FAILURE);
		}

		// Close the connections
		close(connection);
		close(sockfd); */

	return 0;
}
