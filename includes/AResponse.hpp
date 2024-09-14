/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AResponse.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 16:12:57 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/09/14 10:00:47 by mde-sa--         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ARESPONSE_HPP
#define ARESPONSE_HPP

#include "HTTPRequestParser.hpp"
#include "Server.hpp"
#include "Webserv.hpp"

class Server;
class LocationContext;

struct HTTP_Response {
	unsigned short status;
	std::multimap<std::string, std::string> headers;
	std::string body;

	// NOTE: Default constructor to initialize the members
	HTTP_Response() : status(0) {}
};

class AResponse {
   public:
	AResponse(const Server& server, const HTTP_Request& request);
	AResponse(const AResponse& src);
	virtual ~AResponse();

	// Pure virtual method for generating the HTTP response
	virtual std::string generateResponse() = 0;

   protected:
	HTTP_Request _request;
	HTTP_Response _response;
	const Server& _server;
	std::string _locationRoute;

	// Validators
	short checkSize() const;
	short checkMethod() const;
	short checkFile(const std::string& path) const;
	bool hasAutoindex() const;
	bool hasReturn() const;
	bool isDirectory(const std::string& path) const;

	// Response construtors
	void setMatchLocationRoute();
	void setMimeType(const std::string& path);
	void loadCommonHeaders();
	void loadReturn();
	// short loadFile(const std::string& path); // for GET
	short loadDirectoryListing(const std::string& path);
	const std::string loadErrorPage(short status);

	// Utilities
	const std::string getPath() const;
	const std::string assemblePath(const std::string& l,
								   const std::string& r) const;
	const std::string getIndexFile(const std::string& path) const;
	const std::string getResponseStr() const;

   private:
	AResponse();
	const AResponse& operator=(const AResponse& src);
};

#endif
