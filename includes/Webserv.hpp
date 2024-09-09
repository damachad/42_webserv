/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/01 12:00:46 by damachad          #+#    #+#             */
/*   Updated: 2024/09/06 16:03:01 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <arpa/inet.h>	// for inet.aton()
#include <errno.h>		// For errno
#include <fcntl.h>		// open
#include <limits.h>
#include <netinet/in.h>
#include <netinet/in.h>	 // For sockaddr_in
#include <sys/epoll.h>	 // For epoll
#include <sys/socket.h>	 // For socket functions
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>	 // For read
#include <ctime>

#include <cstdlib>	 // For exit() and EXIT_FAILURE
#include <cstring>	 // For memset (?)
#include <fstream>	 // For file streams
#include <iostream>	 // For cout
#include <sstream>

// STL
#include <algorithm>
#include <map>
#include <set>
#include <vector>

// Custom Variables // NOTE: Included here to facilitate .hpp inclusion issues
enum Method { GET = 1, POST, DELETE, UNKNOWN };
enum State { FALSE, TRUE, UNSET };

// Custom Classes
class ServerContext;
class Server;
class Cluster;
class LocationContext;
class ConfigParser;

// Custom Structs
struct Listen;

// Custom .hpp files
#include "Cluster.hpp"
#include "ConfigParser.hpp"
#include "Exceptions.hpp"
#include "HTTPRequestParser.hpp"
#include "Helpers.hpp"
#include "LocationContext.hpp"
#include "Server.hpp"
#include "ServerContext.hpp"
#include "AResponse.hpp"

// Magic numbers		 // TODO: Do we need them?
#define MAX_CONNECTIONS 100	 // TODO: We have to think what should be the max

extern const std::map<short, std::string> STATUS_MESSAGES;

#endif
