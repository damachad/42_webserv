#ifndef CLUSTER_HPP
#define CLUSTER_HPP

#include "Webserv.hpp"

class Server;

class Cluster {
   public:
	Cluster(std::vector<struct Context> servers);
	~Cluster();

	const Server& operator[](unsigned int index) const;

   private:
	std::vector<Server> _servers;

	const Cluster& operator=(const Cluster& copy);
	Cluster(const Cluster& copy);
};

#endif
