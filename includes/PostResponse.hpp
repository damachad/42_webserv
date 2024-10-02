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

struct file {
	std::string name;
	std::string file_name;
	std::string content_type;

	std::string file_contents;
};

class PostResponse : public AResponse {
   public:
	PostResponse(const Server& server, const HTTP_Request& request,
				 int client_fd, int epoll_fd);
	PostResponse(const PostResponse& src);
	~PostResponse();

	std::string generateResponse();

   private:
	PostResponse();
	PostResponse& operator=(const PostResponse& src);

	bool requestHasContentLength();
	short checkBody();
	short extractFile();
	short uploadFile();
	const std::string getBoundary();
	const std::vector<std::multimap<std::string, std::string> >
	getMultipartBody(const std::string& boundary);
	const std::multimap<std::string, std::string> extractFields(
		const std::string& subpart);
	std::string extractFieldValue(const std::string& header,
								  const std::string& field);

	std::vector<std::multimap<std::string, std::string> > _multipart_body;
	std::string _boundary;
	struct file _file_to_upload;

	int _client_fd;
	int _epoll_fd;
};

#endif
