// FileName: ProcessHandler.hpp

#pragma once

#include "EventHandler.hpp"
#include "ProcessState.hpp"

class ProcessState;

class ProcessHandler: public EventHandler {
private:
	static const time_t	_timeout = 30;
	// Attributes
	int				_port;
	ProcessState*	state;
public:
	// Constructors & Destructor
	ProcessHandler(int socket_fd, int port);
	~ProcessHandler();
	// Getters
	int				getPort() const;
	ProcessState*	getState() const;

	// Setters
	void	setState(ProcessState* state);

	// Methods
	void	handle();
	void	timeout();
	time_t	getTimeout() const;
};
