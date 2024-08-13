/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/13 14:49:03 by damachad          #+#    #+#             */
/*   Updated: 2024/08/13 15:10:34 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

int	main(int argc, char **argv)
{
	if (argc != 2)
	{
		// load default conf file ?
		std::cout << "Usage: ./webserv [configuration file]";
		return (1);
	}
	try
	{
		ConfigParser parser(argv[1]);
	}
	catch (std::exception & e)
	{
		std::cerr << e.what() << '\n';
	}
	return (0);
}
