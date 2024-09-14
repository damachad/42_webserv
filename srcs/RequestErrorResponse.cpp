/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestErrorResponse.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mde-sa-- <mde-sa--@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 10:40:06 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/09/14 10:41:03 by mde-sa--         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestErrorResponse.hpp"

RequestErrorResponse::RequestErrorResponse(const Server& server,
										   const HTTP_Request& request)
	: AResponse(server, request) {}

RequestErrorResponse::RequestErrorResponse(const RequestErrorResponse& src)
	: AResponse(src) {}

RequestErrorResponse::~RequestErrorResponse() {}

std::string RequestErrorResponse::generateResponse() {
	std::string response("Hi");

	std::cout << response << std::endl;

	return response;
}
