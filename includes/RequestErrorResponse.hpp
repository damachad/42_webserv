/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestErrorResponse.hpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mde-sa-- <mde-sa--@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 10:38:42 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/09/14 10:41:39 by mde-sa--         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTERRORRESPONSE_HPP
#define REQUESTERRORRESPONSE_HPP

#include "AResponse.hpp"
#include "Webserv.hpp"

class RequestErrorResponse : public AResponse {
   public:
	RequestErrorResponse(const Server& server, const HTTP_Request& request);
	RequestErrorResponse(const RequestErrorResponse& src);
	~RequestErrorResponse();

	std::string generateResponse();

   private:
	RequestErrorResponse();
	RequestErrorResponse& operator=(const RequestErrorResponse& src);
};

#endif
