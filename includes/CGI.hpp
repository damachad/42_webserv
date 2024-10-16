/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/15 13:04:53 by damachad          #+#    #+#             */
/*   Updated: 2024/10/16 15:27:38 by damachad         ###   ########.fr       */
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
	std::string getEnvVar(const char *key);
	std::string fetchCookies();

	// Env setter
	void setSingleEnv(std::vector<std::string> &env, std::string key,
					  std::string envToAdd);
	short setCGIEnv();

	// Parsers
	std::multimap<std::string, std::string> parseCGIHeaders(
		const std::string &headers);
	std::multimap<std::string, std::string> parseRequestHeaders();

	// Utils
	std::string intToString(int value);
	char **vectorToCharArray(const std::vector<std::string> &vec);

   public:
	CGI(HTTP_Request &httpRequest, HTTP_Response &httpResponse,
		const std::string &path);
	~CGI();

	void handleCGIResponse();
	std::string executeCGI(const std::string &scriptPath);
	std::string getHeaderEnvValue(std::string key);
};

#endif
