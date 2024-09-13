/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetResponse.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 15:14:52 by damachad          #+#    #+#             */
/*   Updated: 2024/09/13 16:09:50 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "GetResponse.hpp"

#include "AResponse.hpp"

GetResponse::~GetResponse() {}

GetResponse::GetResponse(const GetResponse& src) : AResponse(src) {}

GetResponse::GetResponse(const Server& server, const HTTP_Request& request)
	: AResponse(server, request) {}

// Loads response with contents of file and sets MIME type
short GetResponse::loadFile(const std::string& path) {
	std::ifstream file(path.c_str());
	if (!file.is_open()) return 500;
	_response.body.assign((std::istreambuf_iterator<char>(file)),
						  (std::istreambuf_iterator<char>()));
	file.close();
	loadCommonHeaders();
	setMimeType(path);
	_response.status = 200;
	return 200;
}

std::string GetResponse::generateResponse() {
	setMatchLocationRoute();
	short status = checkSize();
	if (status != 200) return loadErrorPage(status);
	status = checkMethod();
	if (status != 200) return loadErrorPage(status);
	if (hasReturn()) {
		loadReturn();
		return getResponseStr();
	}
	std::string path = getPath();

	status = checkFile(path);
	if (status != 200) return loadErrorPage(status);
	if (!isDirectory(path)) {
		status = loadFile(path);
		if (status != 200) return loadErrorPage(status);
	} else {  // is a directory
		std::string indexFile = getIndexFile(path);
		if (!indexFile.empty() &&
			!isDirectory(indexFile)) {	// TODO: deal with directory in index?
			status = loadFile(indexFile);
			if (status != 200) return loadErrorPage(status);
		} else if (hasAutoindex()) {
			status = loadDirectoryListing(path);
			if (status != 200) return loadErrorPage(status);
		} else
			loadErrorPage(404);
	}

	return getResponseStr();
}