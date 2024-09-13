/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mde-sa-- <mde-sa--@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 12:54:31 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/09/13 13:10:07 by mde-sa--         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "PostResponse.hpp"

#include "AResponse.hpp"

PostResponse::PostResponse(ServerContext *server, HTTP_Request *request)
	: AResponse(server, request) {}

PostResponse::PostResponse(const PostResponse &src) : AResponse(src) {}

PostResponse::~PostResponse() {}

std::string PostResponse::generateResponse() {
	std::string response("This is the response");

	std::cout << "Hi!" << std::endl;

	return response;
}
