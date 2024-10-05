/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AResponse.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 16:12:57 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/09/17 09:51:37 by damachad         ###   ########.fr       */
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
	const AResponse& operator=(const AResponse& src);
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
	short checkClientBodySize() const;
	short checkMethod() const;
	short checkFile(const std::string& path) const;
	bool hasAutoindex() const;
	bool hasReturn() const;
	bool isDirectory(const std::string& path) const;
  bool isCGI();
	// Response construtors
	void setMatchLocationRoute();
	void setMimeType(const std::string& path);
	void loadCommonHeaders();
	void loadReturn();
	std::string addFileEntry(std::string& name, const std::string& path);
	short loadDirectoryListing(const std::string& path);
	const std::string loadErrorPage(short status);
	const std::string loadContinueMessage();

	// Utilities
	const std::string getPath() const;
	const std::string assemblePath(const std::string& l,
								   const std::string& r) const;
	const std::string getIndexFile(const std::string& path) const;
	const std::string getResponseStr() const;

   private:
	AResponse();
};

#endif
