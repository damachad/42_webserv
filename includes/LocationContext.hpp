/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationContext.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/21 11:37:41 by damachad          #+#    #+#             */
/*   Updated: 2024/08/21 12:34:27 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONTEXT_HPP
# define LOCATIONCONTEXT_HPP

# include "Webserv.hpp"

class LocationContext
{
	private:
		std::string _root;
		std::vector<std::string> _index;
		State _autoIndex;
		long _clientMaxBodySize;
		std::vector<std::string> _tryFiles;
		std::vector<Method> _allowedMethods;
		std::map<short, std::string> _errorPages;
		// std::string _uploadDir;	// Is this necessary ?
		// Later add redirect and cgi related variables
	
	public:

		LocationContext();
		LocationContext(const LocationContext & src);
		~LocationContext();

		LocationContext & operator=(const LocationContext & src);

		// Getters
		std::string getRoot() const;
		std::vector<std::string> getIndex() const;
		State getAutoIndex() const;
		long getClientMaxBodySize() const;
		std::vector<std::string> getTryFiles() const;
		std::vector<Method> getAllowedMethods() const;
		std::map<short, std::string> getErrorPages() const;

		// Setters
		void setRoot(const std::string& root);
		void setIndex(const std::vector<std::string>& index);
		void setAutoIndex(State autoIndex);
		void setClientMaxBodySize(long clientMaxBodySize);
		void setTryFiles(const std::vector<std::string>& tryFiles);
		void setAllowedMethods(const std::vector<Method>& allowedMethods);
		void setErrorPages(const std::map<short, std::string>& errorPages);

		// Methods to add elements
		void addIndex(const std::string& index);
		void addTryFile(const std::string& tryFile);
		void addAllowedMethod(const Method& method);
		void addErrorPage(short code, const std::string& page);
};

#endif