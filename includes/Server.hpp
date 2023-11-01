/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: chmadran <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/10/31 14:40:38 by chmadran          #+#    #+#             */
/*   Updated: 2023/11/01 16:07:58 by chmadran         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

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
# include "ServerResponse.hpp"

class Server {
public:
	Server();
	void start();

protected:
	int server_fd;
	struct sockaddr_in address;

	void setupNetwork();
	void handleClientRequest(int clientSocket);
};

#endif
