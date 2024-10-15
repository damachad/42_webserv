/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/01 12:00:46 by damachad          #+#    #+#             */
/*   Updated: 2024/10/14 19:58:16 by damachad         ###   ########.fr       */
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
#include <signal.h>

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

// TODO: Decide what global definitions we want
// Magic numbers and strings
#define MAX_CONNECTIONS 100
#define MAX_STORAGE_SIZE 100000
#define REG_FILE 0
#define DIRECTORY 1
#define URL_MAX_SIZE 8094
#define SERVER "webserv"

extern const std::map<short, std::string> STATUS_MESSAGES;

extern unsigned int total_used_storage;
extern bool running;

#endif
