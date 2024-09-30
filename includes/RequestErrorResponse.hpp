/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestErrorResponse.hpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 10:38:42 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/09/18 15:06:05 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTERRORRESPONSE_HPP
#define REQUESTERRORRESPONSE_HPP

#include "AResponse.hpp"
#include "Webserv.hpp"

class RequestErrorResponse : public AResponse {
   public:
	RequestErrorResponse(const Server& server, const HTTP_Request& request, short error_status);
	RequestErrorResponse(const RequestErrorResponse& src);
	~RequestErrorResponse();

	std::string generateResponse();

   private:
    short _error_status;
	RequestErrorResponse();
	RequestErrorResponse& operator=(const RequestErrorResponse& src);
};

#endif
