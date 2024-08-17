/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/01 12:00:46 by damachad          #+#    #+#             */
/*   Updated: 2024/08/14 17:28:19 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <errno.h>		 // For errno
#include <fcntl.h>		 // open
#include <netinet/in.h>	 // For sockaddr_in
#include <sys/epoll.h>	 // For epoll
#include <sys/socket.h>	 // For socket functions
#include <sys/types.h>
#include <unistd.h>	 // For read

#include <cstdlib>	 // For exit() and EXIT_FAILURE
#include <cstring>	 // For memset (?)
#include <fstream>	 // For file streams
#include <iostream>	 // For cout
#include <sstream>

// STL
#include <algorithm>
#include <map>
#include <vector>

#include "Cluster.hpp"
#include "ConfigParser.hpp"
#include "Exceptions.hpp"
#include "Helpers.hpp"
#include "Server.hpp"

// Magic numbers
#define MAX_CONNECTIONS 100	 // TODO: We have to think what should be the max

#endif
