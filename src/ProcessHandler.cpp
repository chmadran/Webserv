#include "ProcessHandler.hpp"
#include "ServerManager.hpp"
#include "ReadState.hpp"
#include <iostream>

ProcessHandler::ProcessHandler(int socket_fd, int port): EventHandler(socket_fd), _port(port), state(new ReadState(this, socket_fd))
{}

ProcessHandler::~ProcessHandler()
{
	delete this->state;
}

void ProcessHandler::setState(ProcessState *state)
{
	this->state = state;
}

int ProcessHandler::getPort() const
{
	return this->_port;
}

ProcessState *ProcessHandler::getState() const
{
	return this->state;
}

void ProcessHandler::handle()
{
	this->state->process();
}

void ProcessHandler::timeout()
{
	ServerManager::getInstance()->deleteClient(this->getSocketFd());
}

time_t ProcessHandler::getTimeout() const
{
	return ProcessHandler::_timeout;
}
