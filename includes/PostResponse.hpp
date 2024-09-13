/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mde-sa-- <mde-sa--@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 12:51:01 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/09/13 13:03:23 by mde-sa--         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef POSTRESPONSE_HPP
#define POSTRESPONSE_HPP

#include "AResponse.hpp"
#include "Webserv.hpp"

class PostResponse : public AResponse {
   public:
	PostResponse(ServerContext* server, HTTP_Request* request);
	PostResponse(const PostResponse& copy);
	~PostResponse();

	std::string generateResponse();

   private:
	PostResponse();
	PostResponse& operator=(const PostResponse& copy);
};

#endif
