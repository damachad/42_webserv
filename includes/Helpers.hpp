#ifndef HELPERS_HPP
#define HELPERS_HPP

#include "Webserv.hpp"

Context get_default_conf(int i);
std::string boolToString(bool value);
std::string methodToString(Method method);
std::string int_to_string(int value);
std::ostream& operator<<(std::ostream& outstream,
						 const struct Context configuration);

#endif
