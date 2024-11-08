/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/15 13:04:53 by damachad          #+#    #+#             */
/*   Updated: 2024/10/22 10:05:47 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
#define CGI_HPP

#include "AResponse.hpp"
#include "HTTPRequestParser.hpp"

class CGI {
private:
  const HTTP_Request &_request;
  HTTP_Response &_response;
  const std::string &_path;
  char **_cgiEnv;

  // Checker
  bool isSingleValueHeader(std::string &key);

  // Env getters
  std::string getQueryFields();
  std::string fetchCookies();

  // Env setter
  void setSingleEnv(std::vector<std::string> &env, std::string key,
                    std::string envToAdd);
  short setCGIEnv();

  // Parsers
  std::multimap<std::string, std::string>
  parseCGIHeaders(const std::string &headers);

  // Output related
  void runScript(int *pipeIn, int *pipeOut, const std::string &scriptPath);
  std::string createCgiOutput(pid_t pid, int *pipeOut);

  // Utils
  char **vectorToCharArray(const std::vector<std::string> &vec);
  std::string getServerName();
  std::string getServerPort();

public:
  CGI(HTTP_Request &httpRequest, HTTP_Response &httpResponse,
      const std::string &path);
  ~CGI();

  void handleCGIResponse();
  std::string executeCGI(const std::string &scriptPath);
  std::string getHeaderEnvValue(std::string key);
};

#endif
