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

	void upload_file();
	const std::string get_boundary();
	const std::vector<std::multimap<std::string, std::string> >
	get_multipart_body(const std::string& boundary);
	const std::multimap<std::string, std::string> extract_fields(
		const std::string& subpart);
};

#endif
