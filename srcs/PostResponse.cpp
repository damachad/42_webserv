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
	setMatchLocationRoute();
	unsigned short status;

	status = checkSize();
	if (status != 200) return loadErrorPage(status);

	status = checkClientBodySize();
	if (status != 200) return loadErrorPage(status);

	status = checkMethod();
	if (status != 200) return loadErrorPage(status);

	// Ver Client Body Buffer Size??
	//
	//

	return "HI";
}
