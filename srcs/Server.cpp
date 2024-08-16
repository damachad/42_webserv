#include "Webserv.hpp"

Server::Server(const struct Context& configuration) {
	std::cout << "Created a server!" << std::endl;

	std::cout << configuration << std::endl;
}

Server::~Server() {}
