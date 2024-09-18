/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteResponse.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 13:21:15 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/09/18 14:31:33 by damachad         ###   ########.fr       */
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
// TODO: remove repeated parts with checkFile() or not call checkFile()
short DeleteResponse::deleteFile(const std::string& path) {
	struct stat fileInfo;
	if (stat(path.c_str(), &fileInfo) != 0)
		return 404;  // Not Found
	if (!(fileInfo.st_mode & S_IWUSR)) // No write permission
		return 403;  // Forbidden

	if (std::remove(path.c_str()) == 0)
		return 200;
	else {
		if (errno == EACCES)
			return 403;  // Permission denied
		return 500;  // server error
	}
}

// Checks if a directory is empty
bool DeleteResponse::isDirectoryEmpty(const std::string& path) {
	DIR* dir = opendir(path.c_str());
	if (dir == NULL)
		return false;
	struct dirent* entry;
	int count = 0;
	while ((entry = readdir(dir)) != NULL) {
		// Skip the "." and ".." entries
		if (std::string(entry->d_name) != "." && std::string(entry->d_name) != "..") {
			count++;
			break;  // Directory is not empty, no need to continue
		}
	}
	closedir(dir);
	return (count == 0);
}

// Deletes a directory if it is empty
short DeleteResponse::deleteDirectory(const std::string& path) {
	if (rmdir(path.c_str()) == 0)
		return 200;
	else
		return 403;  // Forbidden
}

// NOTE: No return, index or autoindex logic in Delete
std::string DeleteResponse::generateResponse() {
	setMatchLocationRoute();
	short status;
	status = checkMethod();
	if (status != 200) return loadErrorPage(status);
	std::string path = getPath();

	status = checkFile(path);
	if (status != 200) return loadErrorPage(status);
	if (!isDirectory(path)) {
		status = deleteFile(path);
		if (status != 200) return loadErrorPage(status);
	} else {
		if (isDirectoryEmpty(path)) {
			status = deleteDirectory(path);
			if (status != 200) return loadErrorPage(status);
		} else
			return loadErrorPage(409);  // Conflict (non-empty directory)
	}
	_response.status = 204; // No content (NGINX returns this on a successful Delete)
	return getResponseStr();
}
