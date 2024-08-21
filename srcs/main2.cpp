/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main2.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/13 14:49:03 by damachad          #+#    #+#             */
/*   Updated: 2024/08/21 10:39:52 by damachad         ###   ########.fr       */
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
		parser.printConfigs();
	} catch (std::exception & e)
	{
		std::cerr << e.what();
	}
	return (0);
}
