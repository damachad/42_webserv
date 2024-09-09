/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Helpers.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/19 14:45:07 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/09/09 17:30:30 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HELPERS_HPP
#define HELPERS_HPP

#include "Webserv.hpp"

std::vector<ServerContext> get_default_conf(int i);
std::string boolToString(bool value);
std::string methodToString(Method method);
Method stringToMethod(const std::string& method);
std::string int_to_string(int value);
int string_to_int(const std::string& value);
std::string& getHttpDate();
std::ostream& operator<<(std::ostream& outstream,
						 const struct Context configuration);

#endif
