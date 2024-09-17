/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mde-sa-- <mde-sa--@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 13:21:15 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/09/14 10:41:19 by mde-sa--         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "PostResponse.hpp"

PostResponse::PostResponse(const Server &server, const HTTP_Request &request)
	: AResponse(server, request) {}

PostResponse::PostResponse(const PostResponse &src) : AResponse(src) {}

PostResponse::~PostResponse() {}

std::string PostResponse::generateResponse() {
	std::string response("This is the response");

	std::cout << "Hi!" << std::endl;

	return response;
}
