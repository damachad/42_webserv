/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AResponse.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/03 13:52:46 by damachad          #+#    #+#             */
/*   Updated: 2024/09/03 18:56:39 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

AResponse::AResponse() {}

AResponse::~AResponse() {}

AResponse::AResponse(const HTTP_Request & request)
	: _request(request) {}

AResponse::AResponse(const AResponse & src)
	: _request(src._request) {}

const AResponse & AResponse::operator=(const AResponse & src)
{
	_request = src._request;
}

bool AResponse::isValidSize()
{
	// How to handle multiple Content-Length values ?
	if (_request.header_fields.count("Content-Length") > 1)
		return false; // return a 400 Bad Request
	std::multimap<std::string, std::string>::const_iterator it = _request.header_fields.find("Content-Length");
	if (it != _request.header_fields.end())
	{
		char *endPtr = NULL;
		long size = std::strtol(it->second.c_str(), &endPtr, 10);
		if (*endPtr != '\0')
			return false; // return a 400 Bad Request
		if (size != _request.message_body.size())
			return false; // return a 400 Bad Request
	}
	return true; // mandatory Content-Length header ?
}