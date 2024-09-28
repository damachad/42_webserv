/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestErrorResponse.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/14 10:40:06 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/09/18 15:08:15 by damachad         ###   ########.fr       */
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
	// if (_error_status == 100) return loadContinueMessage;

	return loadErrorPage(_error_status);
}
