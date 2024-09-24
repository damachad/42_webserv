#include <arpa/inet.h>	 // For inet_pton
#include <netdb.h>		 // For gethostbyname
#include <sys/socket.h>	 // For socket functions
#include <unistd.h>		 // For close

#include <cstdlib>	// For exit and atoi
#include <cstring>	// For memset and strlen
#include <fstream>	// For file handling
#include <iostream>

void error(const char *msg) {
	perror(msg);
	exit(1);
}

std::string readFile(const char *filename) {
	std::ifstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Error opening file: " << filename << std::endl;
		exit(1);
	}

	// Read the file contents into a string
	std::string content((std::istreambuf_iterator<char>(file)),
						(std::istreambuf_iterator<char>()));
	file.close();

	return content;
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <IP:Port> <HTTP request file>\n";
		return 1;
	}

	// Extract IP and port from argv[1]
	char *ip = strtok(argv[1], ":");
	char *port_str = strtok(NULL, ":");
	if (!ip || !port_str) {
		std::cerr << "Invalid format for IP and port. Use IP:Port\n";
		return 1;
	}
	int port = atoi(port_str);
	if (port <= 0 || port > 65535) {
		std::cerr << "Invalid port number.\n";
		return 1;
	}

	// Read the HTTP request from the file specified in argv[2]
	std::string request = readFile(argv[2]);

	// Create a socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		error("Error opening socket");
	}

	// Configure the server address
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

	// Convert the IP address
	if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
		error("Invalid IP address");
	}

	// Connect to the server
	if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
		0) {
		error("Error connecting to the server");
	}

	// Send the HTTP request read from the file
	int n = send(sockfd, request.c_str(), request.length(), 0);
	if (n < 0) {
		error("Error writing to socket");
	}

	// Receive the response
	char response[4096];
	memset(response, 0, sizeof(response));
	n = recv(sockfd, response, sizeof(response) - 1, 0);
	if (n < 0) {
		error("Error reading from socket");
	}

	// Print the response
	std::cout << "Response from server:\n" << response << std::endl;

	// Close the socket
	close(sockfd);

	return 0;
}
