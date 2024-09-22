/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mde-sa-- <mde-sa--@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 13:20:57 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/09/14 10:41:55 by mde-sa--         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef POSTRESPONSE_HPP
#define POSTRESPONSE_HPP

#include "AResponse.hpp"
#include "Webserv.hpp"

class PostResponse : public AResponse {
   public:
	PostResponse(const Server& server, const HTTP_Request& request);
	PostResponse(const PostResponse& src);
	~PostResponse();

	std::string generateResponse();

   private:
	PostResponse();
	PostResponse& operator=(const PostResponse& src);

	short checkBody();
	short uploadFile();
	const std::string getBoundary();
	const std::vector<std::multimap<std::string, std::string> >
	getMultipartBody(const std::string& boundary);
	const std::multimap<std::string, std::string> extractFields(
		const std::string& subpart);

	std::vector<std::multimap<std::string, std::string> > _multipart_body;
	std::string _boundary;
};

#endif
