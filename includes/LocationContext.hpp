/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationContext.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 11:37:41 by damachad          #+#    #+#             */
/*   Updated: 2024/08/29 14:13:37 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONTEXT_HPP
#define LOCATIONCONTEXT_HPP

#include "Webserv.hpp"

class LocationContext {
   private:
	std::string _root;
	std::vector<std::string> _index;
	State _autoIndex;
	long _clientMaxBodySize;
	std::vector<std::string> _tryFiles;
	std::set<Method> _allowedMethods;
	std::map<short, std::string> _errorPages;
	std::pair<short, std::string> _return;
	// std::string _uploadDir;	// Is this necessary ?
	// Later add redirect and cgi related variables
	typedef void (LocationContext::*DirectiveHandler)(
		std::vector<std::string> &);
	std::map<std::string, DirectiveHandler> _directiveMap;

   public:
	LocationContext();
	LocationContext(const LocationContext &src);
	~LocationContext();

	LocationContext &operator=(const LocationContext &src);

	// Getters
	std::string getRoot() const;
	std::vector<std::string> getIndex() const;
	State getAutoIndex() const;
	long getClientMaxBodySize() const;
	std::vector<std::string> getTryFiles() const;
	std::set<Method> getAllowedMethods() const;
	std::map<short, std::string> getErrorPages() const;
	std::pair<short, std::string> getReturn() const;

	// Handlers for directives
	void handleRoot(std::vector<std::string> &tokens);
	void handleIndex(std::vector<std::string> &tokens);
	void handleTryFiles(std::vector<std::string> &tokens);
	void handleLimitExcept(std::vector<std::string> &tokens);
	void handleErrorPage(std::vector<std::string> &tokens);
	void handleCliMaxSize(std::vector<std::string> &tokens);
	void handleAutoIndex(std::vector<std::string> &tokens);
	void handleReturn(std::vector<std::string> &tokens);

	void initializeDirectiveMap();
	void processDirective(std::string &line);
};

std::ostream &operator<<(std::ostream &os, const LocationContext &context);

#endif
