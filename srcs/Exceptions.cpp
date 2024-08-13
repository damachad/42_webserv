/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Exceptions.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/13 12:31:46 by damachad          #+#    #+#             */
/*   Updated: 2024/08/13 13:52:43 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

FileReadError::FileReadError(const std::string &str) throw()
{
	_message = "Unable to read from: " + str;
}

const char *FileReadError::what() const throw()
{
	return (_message.c_str());
}

FileReadError::~FileReadError() throw(){}
