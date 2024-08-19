/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main2.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/13 14:49:03 by damachad          #+#    #+#             */
/*   Updated: 2024/08/19 15:44:50 by damachad         ###   ########.fr       */
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
	ConfigParser parser(argv[1]);
	try{
		parser.loadConfigs();
	} catch (std::exception & e)
	{
		std::cerr << e.what();
	}
	parser.printConfigs();
	return (0);
}
