/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main2.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/13 14:49:03 by damachad          #+#    #+#             */
/*   Updated: 2024/09/09 17:28:26 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"
#include "Webserv.hpp"

int main(int argc, char **argv) {
	if (argc != 2) {
		// load default conf file ?
		std::cout << "Usage: ./webserv [configuration file]";
		return (1);
	}

	ConfigParser parser(argv[1]);

	try {
		parser.loadConfigs();
		std::vector<ServerContext> servers = parser.getServers();
		for (std::vector<ServerContext>::const_iterator it = servers.begin();
			 it != servers.end(); ++it)
			std::cout << (*it) << "\n";
		std::string route = "/test";
		unsigned int serverNum = 0;
		parser.printLocationValues(serverNum, route);
	} catch (std::exception &e) {
		std::cerr << e.what();
	}
	return (0);
}
