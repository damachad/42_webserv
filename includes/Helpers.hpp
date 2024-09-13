/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Helpers.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/19 14:45:07 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/09/10 11:03:11 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HELPERS_HPP
#define HELPERS_HPP

class Server;

#include "ServerContext.hpp"
#include "Webserv.hpp"

// Type conversions
std::string boolToString(bool value);
std::string methodToString(Method method);
Method stringToMethod(const std::string& method);

template <typename T>
std::string numberToString(T value) {
	std::stringstream ss;
	ss << value;
	return ss.str();
}

template <typename T>
T stringToNumber(const std::string& value) {
	T result;

	std::stringstream ss(value);

	ss >> result;

	// Check if the conversion was successful
	if (ss.fail() || !ss.eof()) {
		throw std::invalid_argument(
			"Invalid input: could not convert string to " +
			std::string(typeid(T).name()));
	}

	return result;
}

std::string getHttpDate();
std::ostream& operator<<(std::ostream& outstream,
						 const struct Context configuration);
#endif
