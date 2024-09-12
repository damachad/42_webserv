/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Helpers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/19 14:45:02 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/08/21 16:15:09 by mde-sa--         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Helpers.hpp"

std::string boolToString(bool value) { return value ? "True" : "False"; }

std::string methodToString(Method method) {
	switch (method) {
		case GET:
			return "GET";
		case POST:
			return "POST";
		case DELETE:
			return "DELETE";
		default:
			return "UNKNOWN";
	}
}

Method stringToMethod(const std::string& method) {
	if (method == "GET")
		return GET;
	else if (method == "POST")
		return POST;
	else if (method == "DELETE")
		return DELETE;
	else
		return UNKNOWN;
}

std::string int_to_string(int value) {
	std::stringstream ss;
	ss << value;
	return ss.str();
}

int string_to_int(const std::string& value) {
	int result;

	std::stringstream ss(value);

	ss >> result;

	// Check if the conversion was successful
	if (ss.fail() || !ss.eof()) {
		throw std::invalid_argument(
			"Invalid input: could not convert string to int");
	}

	return result;
}

unsigned int stringToUnsignedInt(const std::string& value) {
	unsigned int result;

	std::stringstream ss(value);

	ss >> result;

	// Check if the conversion was successful
	if (ss.fail() || !ss.eof()) {
		throw std::invalid_argument(
			"Invalid input: could not convert string to unsigned int");
	}

	return result;
}

std::ostream& operator<<(std::ostream& outstream,
						 const ServerContext configuration) {
	outstream << "Network Addresses: ";

	std::vector<Listen> networkaddress = configuration.getNetworkAddress();
	for (std::vector<Listen>::const_iterator it = networkaddress.begin();
		 it != networkaddress.end(); it++)
		outstream << (*it).IP << ":" << (*it).port << "\t";
	outstream << std::endl;
	outstream << "Server Name: " << configuration.getServerName()[0]
			  << std::endl;
	outstream << "Index: " << configuration.getIndex()[0] << std::endl;
	outstream << "AutoIndex: " << boolToString(configuration.getAutoIndex())
			  << std::endl;
	outstream << "Client Max Body Size: "
			  << configuration.getClientMaxBodySize() << std::endl;
	outstream << "Allowed methods: ";

	std::set<Method> allowedmethods = configuration.getAllowedMethods();
	for (std::set<Method>::const_iterator it = allowedmethods.begin();
		 it != allowedmethods.end(); it++)
		outstream << methodToString(*it) << " ";
	outstream << std::endl;

	return outstream;
}
