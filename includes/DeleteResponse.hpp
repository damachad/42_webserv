/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteResponse.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 13:20:57 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/09/18 14:31:03 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DELETERESPONSE_HPP
#define DELETERESPONSE_HPP

#include "AResponse.hpp"
#include "Webserv.hpp"

class DeleteResponse : public AResponse {
   public:
	DeleteResponse(const Server& server, const HTTP_Request& request);
	DeleteResponse(const DeleteResponse& src);
	~DeleteResponse();

	bool isDirectoryEmpty(const std::string& path);

	std::string generateResponse();
	short deleteFile(const std::string& path);
	short deleteDirectory(const std::string& path);

   private:
	DeleteResponse();
	DeleteResponse& operator=(const DeleteResponse& src);
};

#endif
