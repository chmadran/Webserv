#include "AcceptHandler.hpp"
#include "ServerManager.hpp"

AcceptHandler::AcceptHandler(int socket_fd, int port): EventHandler(socket_fd, port)
{}

AcceptHandler::~AcceptHandler()
{}

void AcceptHandler::handle()
{
	int					client_fd;
	struct sockaddr_in	client_address;

	errno = 0;
	std::cout << "✅ New connection on port " << this->getPort() << std::endl;
	client_fd = accept(this->getSocketFd(), (struct sockaddr*)&client_address, (socklen_t []){sizeof(client_address)});
	if (client_fd == -1)
		return perror(SCSTR(__FILE__ << ":" << __LINE__ << " accept() failed"));

	ServerManager::getInstance()->addClient(client_fd, this->getPort());
}
