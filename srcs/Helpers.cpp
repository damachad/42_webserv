/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Helpers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/19 14:45:02 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/10/03 11:07:27 by damachad         ###   ########.fr       */
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

// Returns lowercase version of strin
std::string toLower(const std::string& str) {
	std::string lower_str = str;
	for (std::string::iterator it = lower_str.begin(); it != lower_str.end();
		 ++it) {
		*it = std::tolower(static_cast<unsigned char>(*it));
	}
	return lower_str;
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

// Function to convert a 3-letter month abbreviation into a month number (0-11)
static int getMonthFromString(const std::string& month_str) {
	const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	for (int i = 0; i < 12; ++i) {
		if (month_str == months[i]) {
			return i; // Return month index (0-11)
		}
	}
	return -1; // Error, invalid month
}

// Parse a time string to time_t
time_t parseTime(const std::string& http_time) {
	struct tm t;
	memset(&t, 0, sizeof(struct tm));

	char month_str[4]; // To hold the 3-letter month abbreviation
	int day, year, hour, minute, second;

	// Example: "Wed, 21 Oct 2015 07:28:00 GMT"
	if (sscanf(http_time.c_str(), "%*3s, %d %3s %d %d:%d:%d GMT", &day, month_str, &year, &hour, &minute, &second) != 6)
		return -1; // Parsing error


	// Convert the 3-letter month abbreviation to a month number (0-11)
	int month = getMonthFromString(month_str);
	if (month == -1) return -1; // Invalid month

	// Set tm structure
	t.tm_mday = day;
	t.tm_mon = month;
	t.tm_year = year - 1900; // tm_year is years since 1900
	t.tm_hour = hour;
	t.tm_min = minute;
	t.tm_sec = second;
	t.tm_isdst = 0; // Not considering daylight saving time

	// Convert to time_t (UTC time since epoch)
	time_t parsed_time = timegm(&t); // Use timegm to avoid timezone adjustments
	return parsed_time;
}

std::ostream& operator<<(std::ostream& outstream, const Server configuration) {
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
