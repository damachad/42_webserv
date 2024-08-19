/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Helpers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mde-sa-- <mde-sa--@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/19 14:45:02 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/08/19 15:06:22 by mde-sa--         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

std::vector<struct Context> get_default_conf(int i) {
	std::vector<struct Context> default_conf(0);

	for (int j = 0; j < i; j++) {
		struct Listen listen;
		listen.port = "8080";
		listen.IP = "";

		struct Context conf;
		conf.network_address.push_back(listen);
		conf.serverName.push_back("example.com");
		conf.index.push_back("index.html");
		conf.autoIndex = false;
		conf.clientMaxBodySize = 200000;
		conf.uploadDir = "None";
		conf.allowedMethods.push_back(GET);
		conf.allowedMethods.push_back(POST);
		conf.allowedMethods.push_back(DELETE);

		default_conf.push_back(conf);
	}

	default_conf[1].network_address[0].IP = "localhost";
	default_conf[1].network_address[0].port = "8081";

	default_conf[2].network_address[0].IP = "0.0.0.0";
	default_conf[2].network_address[0].port = "8082";

	default_conf[3].network_address[0].IP = "172.21.187.192";
	default_conf[3].network_address[0].port = "8083";

	return default_conf;
}

std::string boolToString(bool value) {
	if (value)
		return "True";
	else
		return "False";
}

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

std::string int_to_string(int value) {
	std::stringstream ss;
	ss << value;
	return ss.str();
}

int string_to_int(const std::string& value) {
	int result;

	std::stringstream ss(value);

	ss >> result;
	return result;
}

std::ostream& operator<<(std::ostream& outstream,
						 const struct Context configuration) {
	outstream << "Network Addresses: ";
	for (std::vector<Listen>::const_iterator it =
			 configuration.network_address.begin();
		 it != configuration.network_address.end(); it++)
		outstream << (*it).IP << ":" << (*it).port;
	outstream << std::endl;
	outstream << "Server Name: " << configuration.serverName[0] << std::endl;
	outstream << "Index: " << configuration.index[0] << std::endl;
	outstream << "AutoIndex: " << boolToString(configuration.autoIndex)
			  << std::endl;
	outstream << "Client Max Body Size: " << configuration.clientMaxBodySize
			  << std::endl;
	outstream << "Upload Directory: " << configuration.uploadDir << std::endl;
	outstream << "Allowed methods: ";
	for (std::vector<Method>::const_iterator it =
			 configuration.allowedMethods.begin();
		 it != configuration.allowedMethods.end(); it++)
		outstream << methodToString(*it) << " ";
	outstream << std::endl;

	return outstream;
}
