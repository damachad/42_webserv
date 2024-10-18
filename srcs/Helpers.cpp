/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Helpers.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/19 14:45:02 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/10/17 16:42:43 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Helpers.hpp"

bool running = true;

// Sets global flag to false when a SIGINT is caught
void sigIntHandler(int signum) {
	if (signum == SIGINT)
		running = false;
}

// Type conversions

// Returns string literal of a bool
std::string boolToString(bool value) { return value ? "True" : "False"; }

// Returns string based on its Method enum
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

// Returns Method enum based on its string
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

// Returns lowercase version of string
std::string toLower(const std::string& str) {
	std::string lower_str = str;
	for (std::string::iterator it = lower_str.begin(); it != lower_str.end();
		 ++it) {
		*it = std::tolower(static_cast<unsigned char>(*it));
	}
	return lower_str;
}

// Returns http format date, e.g. "Thu, 05 Sep 2024 12:34:56 GMT"
std::string getHttpDate() {
	// Get the current time
	std::time_t now = std::time(0);
	// Convert to UTC (GMT)
	std::tm* gmt = std::gmtime(&now);
	char buffer[35];
	// Format to "Thu, 05 Sep 2024 12:34:56 GMT"
	std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmt);

	return std::string(buffer);
}

// Converts a 3-letter month abbreviation into its index (0-11)
static int getMonthFromString(const std::string& month_str) {
	const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	for (int i = 0; i < 12; ++i) {
		if (month_str == months[i]) {
			return i;
		}
	}
	return -1;
}

// Parse a time string to time_t
time_t parseTime(const std::string& http_time) {
	struct tm t;
	memset(&t, 0, sizeof(struct tm));

	char month_str[4];
	int day, year, hour, minute, second;

	// Example: "Wed, 21 Oct 2015 07:28:00 GMT"
	if (sscanf(http_time.c_str(), "%*3s, %d %3s %d %d:%d:%d GMT", &day, month_str, &year, &hour, &minute, &second) != 6)
		return -1;

	int month = getMonthFromString(month_str);
	if (month == -1) return -1;

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
