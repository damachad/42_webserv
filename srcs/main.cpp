/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/01 11:53:15 by damachad          #+#    #+#             */
/*   Updated: 2024/08/16 15:51:29 by mde-sa--         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <netinet/in.h>

#include "Webserv.hpp"

struct Context get_default_conf(int i);

int main() {
	// Sample configuration vector, supposed to mimic David's final output
	std::vector<struct Context> configuration_vector;
	for (int i = 0; i < 10; i++)
		configuration_vector.push_back(get_default_conf(i));

	// Initializes the Server Cluster
	Cluster server_cluster(configuration_vector);

	// Attempts to Setup the Cluster
	try {
		server_cluster.setup_cluster();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}

	std::cout << server_cluster;

	return 0;
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
