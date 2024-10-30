/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 13:20:57 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/10/18 16:34:35 by damachad         ###   ########.fr       */
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
	PostResponse(const Server& server, HTTP_Request& request, int client_fd,
				 int epoll_fd);
	PostResponse(const PostResponse& src);
	~PostResponse();

	std::string generateResponse();

   private:
	PostResponse();
	PostResponse& operator=(const PostResponse& src);

	// Functions to parse the remainder of the body
	unsigned short parseHTTPBody();
	bool send100Continue();
	bool readBody();
	void readContentLength();
	void removeFirstChunk();
	void readChunks();
	ssize_t readChunkSizeFromSocket();
	int skipTrailingCRLF();
	bool requestHasHeader(const std::string& header);
	short checkBody();
	short extractFile();
	short uploadFile();
	short checkFormData();
	const std::string getBoundary();
	const std::vector<std::multimap<std::string, std::string> >
	getMultipartBody(const std::string& boundary);
	const std::multimap<std::string, std::string> extractFields(
		const std::string& subpart);
	std::string extractFieldValue(const std::string& header,
								  const std::string& field);

	std::vector<std::multimap<std::string, std::string> > _multipart_body;
	std::map<int, std::string> _file_buffer;
	std::string _boundary;
	struct file _file_to_upload;

	int _client_fd;
	int _epoll_fd;
};

#endif
