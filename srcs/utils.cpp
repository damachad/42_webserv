/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/16 15:52:46 by damachad          #+#    #+#             */
/*   Updated: 2024/08/16 16:47:09 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Webserv.hpp"

std::string	stringToLower(std::string str)
{
	std::string lower;
	std::string::iterator it;
	for (it = str.begin(); it != str.end(); it++)
		lower.push_back(tolower(*it));
	return (lower);
}

std::string	stringToUpper(std::string str)
{
	std::string upper;
	std::string::iterator it;
	for (it = str.begin(); it != str.end(); it++)
		upper.push_back(toupper(*it));
	return (upper);
}
