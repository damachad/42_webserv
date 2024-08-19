/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Helpers.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mde-sa-- <mde-sa--@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/19 14:45:07 by mde-sa--          #+#    #+#             */
/*   Updated: 2024/08/19 14:45:08 by mde-sa--         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
