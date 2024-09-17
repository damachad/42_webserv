/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteResponse.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mde-sa-- <mde-sa--@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 13:20:57 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/09/13 15:48:14 by mde-sa--         ###   ########.fr       */
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

	std::string generateResponse();

   private:
	DeleteResponse();
	DeleteResponse& operator=(const DeleteResponse& src);
};

#endif
