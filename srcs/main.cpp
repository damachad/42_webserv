/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/01 11:53:15 by damachad          #+#    #+#             */
/*   Updated: 2024/08/01 12:10:21 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "defines.hpp"

int main() {
    // Create a socket (IPv4, TCP)
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        std::cout << "Failed to create socket. errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }

    // Listen to port 9999 on any address
    struct sockaddr_in sockaddr;
    std::memset(&sockaddr, 0, sizeof(sockaddr)); // Clear the struct
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(9999); // htons is necessary to convert a number to network byte order

    if (bind(sockfd, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
        std::cout << "Failed to bind to port 9999. errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }

    // Start listening. Hold at most 10 connections in the queue
    if (listen(sockfd, 10) < 0) {
        std::cout << "Failed to listen on socket. errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }

    // Grab a connection from the queue
    socklen_t addrlen = sizeof(sockaddr);
    int connection = accept(sockfd, (struct sockaddr*)&sockaddr, &addrlen);
    if (connection < 0) {
        std::cout << "Failed to grab connection. errno: " << errno << std::endl;
        exit(EXIT_FAILURE);
    }

    // Read from the connection
    char buffer[100];
    ssize_t bytesRead = read(connection, buffer, sizeof(buffer) - 1);
    if (bytesRead < 0) {
        std::cout << "Failed to read from connection. errno: " << errno << std::endl;
        close(connection);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    buffer[bytesRead] = '\0'; // Null-terminate the buffer
    std::cout << "The message was: " << buffer << std::endl;

    // Send a message to the connection
    std::string response = "Good talking to you\n";
    ssize_t bytesSent = send(connection, response.c_str(), response.size(), 0);
    if (bytesSent < 0) {
        std::cout << "Failed to send response. errno: " << errno << std::endl;
        close(connection);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Close the connections
    close(connection);
    close(sockfd);

    return 0;
}
