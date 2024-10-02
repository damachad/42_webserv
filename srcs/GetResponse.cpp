/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetResponse.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 15:14:52 by damachad          #+#    #+#             */
/*   Updated: 2024/09/18 11:16:18 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "GetResponse.hpp"

#include "AResponse.hpp"
#include "CGI.hpp"

GetResponse::~GetResponse() {}

GetResponse::GetResponse(const GetResponse &src) : AResponse(src) {}

GetResponse::GetResponse(const Server &server, const HTTP_Request &request)
    : AResponse(server, request) {}

// Loads response with contents of file and sets MIME type
short GetResponse::loadFile(std::string &path) {
  std::cout << "!!URI: " << _request.uri << "\n"; // TESTE
  if (_request.uri.length() > 3 &&
      _request.uri.substr(_request.uri.length() - 3) == ".py") {
    std::cout << "!!ENTROU\n"; // TESTE
    CGI cgi(_request, _response, path);
    cgi.handleCGIResponse();
  } else {
    std::ifstream file(path.c_str());
    if (!file.is_open())
      return 500;
    _response.body.assign((std::istreambuf_iterator<char>(file)),
                          (std::istreambuf_iterator<char>()));
    file.close();
    setMimeType(path);
  }
  loadCommonHeaders();
  _response.status = 200;
  return 200;
}

std::string GetResponse::generateResponse() {
  setMatchLocationRoute();
  short status;
  // status = checkSize();
  // if (status != 200) return loadErrorPage(status);
  status = checkMethod();
  if (status != 200)
    return loadErrorPage(status);
  if (hasReturn()) {
    loadReturn();
    return getResponseStr();
  }
  std::string path = getPath();

  status = checkFile(path);
  std::cout << "!!STATUS: " << status << std::endl; // TESTE
  if (status != 200)
    return loadErrorPage(status);
  if (!isDirectory(path)) {
    status = loadFile(path);
    if (status != 200)
      return loadErrorPage(status);
  } else { // is a directory
    std::string indexFile = getIndexFile(path);
    if (!indexFile.empty() &&
        !isDirectory(indexFile)) { // TODO: deal with directory in index?
      status = loadFile(indexFile);
      if (status != 200)
        return loadErrorPage(status);
    } else if (hasAutoindex()) {
      status = loadDirectoryListing(path);
      if (status != 200)
        return loadErrorPage(status);
    } else
      loadErrorPage(404);
  }

  return getResponseStr();
}
