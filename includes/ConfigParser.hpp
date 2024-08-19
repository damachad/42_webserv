/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/12 14:13:04 by damachad          #+#    #+#             */
/*   Updated: 2024/08/19 14:54:39 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "Webserv.hpp"

enum Method { GET = 1, POST, DELETE };

struct Listen {
	std::string IP;
	std::string port;
};

struct Context {
	std::vector<Listen> network_address;
	std::vector<std::string> serverName;
	std::string root;
	std::vector<std::string> index;
	bool autoIndex;
	unsigned long clientMaxBodySize;
	std::string uploadDir;	// Is this necessary ?
	std::vector<std::string> tryFiles;
	std::vector<Method> allowedMethods;
	std::map<short, std::string> errorPages;
	std::map<std::string, Context> locations;

	// Later add redirect and cgi related variables
};

class ConfigParser {
   private:
	// Function pointer type for handlers
	typedef void (ConfigParser::*DirectiveHandler)(
		Context &, const std::vector<std::string> &);
	std::string _configFile;
	std::vector<Context> _servers;
	std::map<std::string, DirectiveHandler> _directiveMap;

   public:
	ConfigParser();
	ConfigParser(const ConfigParser &src);
	ConfigParser(const std::string &file);
	~ConfigParser();
	ConfigParser &operator=(const ConfigParser &src);

	void loadConfigs();
	void loadDefaults();
	void printConfigs();
	void printContext(Context context);
	void initializeDirectiveMap();
	void trimOuterSpaces(std::string &s);
	void trimComments(std::string &s);
	void loadIntoContext(std::vector<std::string> &blocks);
	size_t advanceBlock(std::string content, size_t start);
	std::vector<std::string> splitServerBlocks(std::string content);
	std::vector<std::string> tokenizeLine(std::string line);
	void processLocation(Context &server, std::string block, size_t start,
						 size_t end);
	void processDirective(Context &server, std::string &line);
	std::vector<Context> getServers(void);

	void handleListen(Context &context, const std::vector<std::string> &tokens);
	void handleServerName(Context &context,
						  const std::vector<std::string> &tokens);
	void handleRoot(Context &context, const std::vector<std::string> &tokens);
	void handleIndex(Context &context, const std::vector<std::string> &tokens);
	void handleLimitExcept(Context &context,
						   const std::vector<std::string> &tokens);
	void handleTryFiles(Context &context,
						const std::vector<std::string> &tokens);
	void handleErrorPage(Context &context,
						 const std::vector<std::string> &tokens);
	void handleCliMaxSize(Context &context,
						  const std::vector<std::string> &tokens);
	void handleAutoIndex(Context &context,
						 const std::vector<std::string> &tokens);
};

// Utils
std::string stringToLower(std::string str);
std::string stringToUpper(std::string str);

#endif
