/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetResponse.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: damachad <damachad@student.42porto.com>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 14:51:25 by damachad          #+#    #+#             */
/*   Updated: 2024/09/24 13:33:49 by damachad         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GETRESPONSE_HPP
#define GETRESPONSE_HPP

#include "AResponse.hpp"

class GetResponse : public AResponse {
   public:
	GetResponse(const Server& server, const HTTP_Request& request);
	GetResponse(const GetResponse& src);
	~GetResponse();

	std::string generateResponse();
	short loadFile(const std::string& path);

   private:
	GetResponse();
	GetResponse& operator=(const GetResponse& src);
};

#endif
