/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteResponse.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mde-sa-- <mde-sa--@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 13:21:15 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/09/13 15:47:34 by mde-sa--         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "DeleteResponse.hpp"

#include "AResponse.hpp"

DeleteResponse::DeleteResponse(const Server& server,
							   const HTTP_Request& request)
	: AResponse(server, request) {}

DeleteResponse::DeleteResponse(const DeleteResponse& src) : AResponse(src) {}

DeleteResponse::~DeleteResponse() {}

std::string DeleteResponse::generateResponse() {
	std::string response("This is the response");

	std::cout << "Hi!" << std::endl;

	return response;
}
