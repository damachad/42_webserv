#ifndef SERVER_HPP
#define SERVER_HPP

#include "Webserv.hpp"

class Server {
   public:
	Server(const struct Context& configuration);
	~Server();
};

#endif
