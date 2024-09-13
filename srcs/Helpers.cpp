/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Helpers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/19 14:45:02 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/09/10 11:09:57 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Helpers.hpp"

// Type conversions
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

// returns http format date, e.g. "Thu, 05 Sep 2024 12:34:56 GMT"
std::string getHttpDate() {
	// Get the current time in seconds since the Epoch
	std::time_t now = std::time(0);

	// Convert to UTC (GMT)
	std::tm* gmt = std::gmtime(&now);

	// Buffer to hold the formatted date string
	char buffer[35];

	// Format to "Thu, 05 Sep 2024 12:34:56 GMT"
	std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmt);

	return std::string(buffer);
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
