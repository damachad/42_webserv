/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetResponse.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 15:14:52 by damachad          #+#    #+#             */
/*   Updated: 2024/10/03 13:57:57 by damachad         ###   ########.fr       */
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
	// std::multimap<std::string, std::string>headers = _request.header_fields;
	// std::multimap<std::string, std::string>::const_iterator it_modified = headers.find("if-modified-since"); 
	// NOTE: headers with spaces not parsed for this
	// if (it_modified != headers.end()) {
	// 	std::string last_modified = getLastModificationDate(path);
	// 	if (parseTime(it_modified->second) >= parseTime(last_modified))
	// 		return 304;
	// }
	_response.body.assign((std::istreambuf_iterator<char>(file)),
						  (std::istreambuf_iterator<char>()));
	file.close();
	loadCommonHeaders();
	// If download in URI, GET request will download file
	if (_request.uri.find("download") != std::string::npos) {
		size_t posSlash = _request.uri.find_last_of("/");
		std::string fileName = _request.uri.substr(posSlash + 1);
		if (fileName.empty()) fileName = "download";
		_response.headers.insert(
			std::make_pair(std::string("Content-Disposition"), std::string("attachment; filename=\"" + fileName + "\"")));
	}
	setMimeType(path);
	return 200;
}

std::string GetResponse::generateResponse() {
	setMatchLocationRoute();
	short status = checkMethod();
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
			(checkFile(indexFile) == 200)) {
			status = loadFile(indexFile);
			if (status != 200) return loadErrorPage(status);
		} else if (hasAutoindex()) {
			status = loadDirectoryListing(path);
			if (status != 200) return loadErrorPage(status);
		} else
			loadErrorPage(403); // Forbiden
	}

	return getResponseStr();
}
