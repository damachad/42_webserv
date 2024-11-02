/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestErrorResponse.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 10:40:06 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/11/02 11:34:54 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "RequestErrorResponse.hpp"

RequestErrorResponse::RequestErrorResponse(const Server& server,
										   const HTTP_Request& request,
										   short error_status)
	: AResponse(server, request), _error_status(error_status) {}

RequestErrorResponse::RequestErrorResponse(const RequestErrorResponse& src)
	: AResponse(src), _error_status(src._error_status) {}

RequestErrorResponse::~RequestErrorResponse() {}

std::string RequestErrorResponse::generateResponse() {
	return loadErrorPage(_error_status);
}
