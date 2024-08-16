#include "Cluster.hpp"

#include "Server.hpp"

// TODO: Throw error if no server list provided
Cluster::Cluster(std::vector<struct Context> servers) {
	if (servers.size() == 0) {
		std::cout << "Error: No server struct provided";
		return;
	}
	for (std::vector<struct Context>::iterator it = servers.begin();
		 it != servers.end(); it++)
		_servers.push_back(Server(*it));
}

Cluster::~Cluster() {}

// TODO: Throw error if index out of bounds
const Server& Cluster::operator[](unsigned int index) const {
	if (index >= _servers.size()) {
		std::cout << "Indexing error!" << std::endl;
		return _servers[0];	 // Error to fix
	}

	return _servers[index];
}

const Cluster& Cluster::operator=(const Cluster& copy) {
	(void)copy;
	return copy;
	// Nothing is supposed to happen: private function!
}

Cluster::Cluster(const Cluster& copy) {
	(void)copy;
	// Nothing is supposed to happen: private function!
}
