/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AResponse.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 16:12:57 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/09/04 18:59:17 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ARESPONSE_HPP
#define ARESPONSE_HPP

#include "Webserv.hpp"

struct HTTP_Request;
class ServerContext;
class LocationContext;

class AResponse {
   public:
	AResponse(const ServerContext & server, const HTTP_Request & request);
	AResponse(const AResponse & src);
	virtual ~AResponse();
	const AResponse & operator=(const AResponse & src);

	// Pure virtual method for generating the HTTP response
	virtual std::string generateResponse() = 0;

   protected:
	HTTP_Request _request;
	ServerContext _server;
	std::map<short, std::string> _statusMessages;

	std::string getStatusMessage(short code) const;
	const std::string & getMatchLocationRoute(const std::string & uri);
	const std::string & getPath(const std::string & locationRoute, const std::string & uri);
	
	// return status code
	short isValidSize() const;

   private:
    void initializeStatusMessages();
	AResponse();
};

#endif
