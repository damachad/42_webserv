/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AResponse.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 16:12:57 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/09/06 16:27:14 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ARESPONSE_HPP
#define ARESPONSE_HPP

#include "Webserv.hpp"

struct HTTP_Request;
class ServerContext;
class LocationContext;

struct HTTP_Response {
	short status;

	std::multimap<std::string, std::string> headers;

	std::string body;
};

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
	HTTP_Response _response;
	ServerContext _server;
	std::string	_locationRoute;

	void getMatchLocationRoute();
	const std::string & getPath();

	void checkSize() const;
	void checkMethod() const;
	void checkReturn() const;

	std::string & getErrorPage(short status);

	std::string & getResponseStr() const;

   private:
	AResponse();
};

#endif
