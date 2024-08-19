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
	const std::string get_hostname() const;
	const std::vector<int> get_ports() const;
	const std::vector<int> get_listening_sockets() const;

   private:
	// Server's hostname
	const std::string _hostname;

	// Server's ports
	const std::vector<int> _ports;

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
