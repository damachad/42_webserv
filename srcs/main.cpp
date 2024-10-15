/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/01 11:53:15 by damachad          #+#    #+#             */
/*   Updated: 2024/10/15 09:45:27 by damachad         ###   ########.fr       */
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
	signal(SIGINT, sigIntHandler);
	ConfigParser parser("conf/default2.conf");
	if (argc == 2) parser = ConfigParser(argv[1]);

	std::vector<Server>
		servers;  // Defined outside the try block so it can be used later

	try {
		parser.loadConfigs();
		servers = parser.getServers();
	} catch (std::exception& e) {
		std::cerr << e.what();
		return 1;
	}

	// Initializes the Server Cluster
	Cluster server_cluster(servers);

	if (server_cluster.hasDuplicateVirtualServers()) {
		std::cerr << "Invalid conf file. Repeated IP/Port/ServerName configs"
				  << std::endl;
		return 1;
	}

	// Attempts to Setup the Cluster
	try {
		server_cluster.setupCluster();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}

	// Runs the server on an infinite loop
	try {
		server_cluster.run();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
	std::cout << "\nThank you for using Webserv.\n";

	return 0;
}
