/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AResponse.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 16:12:57 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/09/10 18:41:30 by damachad         ###   ########.fr       */
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
	AResponse(const ServerContext& server, const HTTP_Request& request);
	AResponse(const AResponse& src);
	virtual ~AResponse();
	const AResponse& operator=(const AResponse& src);

	// Pure virtual method for generating the HTTP response
	virtual std::string generateResponse() = 0;

   protected:
	HTTP_Request _request;
	HTTP_Response _response;
	ServerContext _server;
	std::string _locationRoute;

	void setMatchLocationRoute();
	void setMimeType(const std::string& path);
	const std::string& getPath() const;

	short checkSize() const;
	short checkMethod() const;
	short checkFile(const std::string& path) const;
	bool isDirectory(const std::string& path) const;
	bool hasAutoindex() const;
	short loadFile(const std::string& path);
	bool hasReturn();
	void loadCommonHeaders();
	short loadDirectoryListing(const std::string& path);

	std::string assemblePath(const std::string& l, const std::string& r) const;
	std::string getIndexFile(const std::string& path) const;
	std::string& getResponseStr() const;
	std::string& generateErrorResponse(short status);

   private:
	AResponse();
};

#endif
