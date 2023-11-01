/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerResponse.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chmadran <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/01 15:52:18 by chmadran          #+#    #+#             */
/*   Updated: 2023/11/01 16:04:20 by chmadran         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERRESPONSE_HPP
# define SERVERRESPONSE_HPP

# include <string>
# include <vector>
# include <sys/socket.h>
# include <netinet/in.h>
# include <unistd.h>
# include <cstdlib>
# include <cstring>
# include <cstring>
# include <fstream>
# include <iostream>
# include <algorithm>
# include "ClientRequest.hpp"

class ServerResponse {
	public:
		ServerResponse();
		
		void process(const ClientRequest& request, int clientSocket);
		std::string readFileContent(const std::string& filePath);
		void sendHttpResponse(int clientSocket, const std::string& content);
};

#endif