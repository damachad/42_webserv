/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/01 12:00:46 by damachad          #+#    #+#             */
/*   Updated: 2024/09/13 20:04:31 by mde-sa--         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <arpa/inet.h>	// for inet.aton()
#include <dirent.h>
#include <errno.h>	// For errno
#include <fcntl.h>	// open
#include <limits.h>
#include <netinet/in.h>
#include <netinet/in.h>	 // For sockaddr_in
#include <sys/epoll.h>	 // For epoll
#include <sys/socket.h>	 // For socket functions
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>	 // For read

#include <cctype>	// For islanum
#include <cstdlib>	// For exit() and EXIT_FAILURE
#include <cstring>	// For memset (?)
#include <ctime>
#include <fstream>	 // For file streams
#include <iomanip>	 // For iomanip for encoding
#include <iostream>	 // For cout
#include <limits>
#include <sstream>
#include <typeinfo>	 // For typeid

// STL
#include <algorithm>
#include <map>
#include <set>
#include <vector>

// Custom Variables // NOTE: Included here to facilitate .hpp inclusion issues
enum Method { GET = 1, POST, DELETE, UNKNOWN };
enum State { FALSE, TRUE, UNSET };

// Magic numbers		 // TODO: Do we need them?
#define MAX_CONNECTIONS 100	 // TODO: We have to think what should be the max
#define REG_FILE 0
#define DIRECTORY 1
#define URL_MAX_SIZE 8094
#define SERVER "webserv"

extern const std::map<short, std::string> STATUS_MESSAGES;

#endif
