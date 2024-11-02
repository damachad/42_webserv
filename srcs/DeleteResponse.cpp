/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteResponse.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 13:21:15 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/11/02 11:47:59 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "DeleteResponse.hpp"

#include "AResponse.hpp"

DeleteResponse::DeleteResponse(const Server& server,
							   const HTTP_Request& request)
	: AResponse(server, request) {}

DeleteResponse::DeleteResponse(const DeleteResponse& src) : AResponse(src) {}

DeleteResponse::~DeleteResponse() {}

// Attempts to delete file
short DeleteResponse::deleteFile(const std::string& path) {
	struct stat fileInfo;
	if (stat(path.c_str(), &fileInfo) != 0) return NOT_FOUND;
	if (!(fileInfo.st_mode & S_IWUSR))	// No write permission
		return FORBIDDEN;
	size_t fileSize = fileInfo.st_size;
	if (std::remove(path.c_str()) == 0) {
		total_used_storage -= fileSize;
		return OK;
	} else {
		if (errno == EACCES) return FORBIDDEN;
		return INTERNAL_SERVER_ERROR;
	}
}

// Checks if a directory is empty
bool DeleteResponse::isDirectoryEmpty(const std::string& path) {
	DIR* dir = opendir(path.c_str());
	if (dir == NULL) return false;
	struct dirent* entry;
	int count = 0;
	while ((entry = readdir(dir)) != NULL) {
		// Skip the "." and ".." entries
		if (std::string(entry->d_name) != "." &&
			std::string(entry->d_name) != "..") {
			count++;
			break;
		}
	}
	closedir(dir);
	return (count == 0);
}

// Deletes a directory if it is empty
short DeleteResponse::deleteDirectory(const std::string& path) {
	if (rmdir(path.c_str()) == 0)
		return OK;
	else
		return FORBIDDEN;
}

// Generates Delete response as a string
// NOTE: No return, index or autoindex logic in Delete
std::string DeleteResponse::generateResponse() {
	setMatchLocationRoute();
	short status;
	status = checkMethod();
	if (status != OK) return loadErrorPage(status);
	std::string path = getPath();

	status = checkFile(path);
	if (status != OK) return loadErrorPage(status);
	if (!isDirectory(path)) {
		status = deleteFile(path);
		if (status != OK) return loadErrorPage(status);
	} else {
		if (isDirectoryEmpty(path)) {
			status = deleteDirectory(path);
			if (status != OK) return loadErrorPage(status);
		} else
			return loadErrorPage(CONFLICT);	 // Non-empty directory
	}
	_response.status = NO_CONTENT;	// NGINX status on a successful Delete
	return getResponseStr();
}
