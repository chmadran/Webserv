#include "CgiStrategy.hpp"
#include "Response.hpp"
#include "ServerManager.hpp"
#include <iostream>

/************************************************************
 *					CONSTRUCTOR/DESTRUCTOR					*
 ************************************************************/

CgiStrategy::CgiStrategy(ResponseBuildState *state, std::string cgiInterpreter, int method, std::string body): ResponseBuildingStrategy(state), _timeout(false), _cgiBody(body), _interpreter(cgiInterpreter), _method(method), _pid(-1)
{
	this->_fd[READ] = -1;
	this->_fd[WRITE] = -1;
}

CgiStrategy::~CgiStrategy()
{
	if (this->_fd[READ] != -1)
		close(this->_fd[READ]);
	if (this->_fd[WRITE] != -1)
		close(this->_fd[WRITE]);
}

void CgiStrategy::buildResponse()
{
	if (this->_scriptName.empty())
	{
		int		error;

		setPath();
		if (this->getError())
			return;
		setEnv();
		convertEnv();
		error = executeScript();
		freeEnvp();
		close(this->_fd[WRITE]);
		this->_fd[WRITE] = -1;
		if (error)
		{
			this->setError(500);
			return;
		}
		ServerManager::getInstance()->addCgiChild(this->_fd[READ], this->getState()->getSocketFd(), *this->getState()->getHandler());
		ServerManager::getInstance()->ignoreClient(this->getState()->getSocketFd());
	}
	else
	{
		int		status;
		char	buf[4];
		bool	is_finished;
		int		read_ret;

		is_finished = waitpid(this->_pid, &status, WNOHANG);
		if (is_finished && !WIFEXITED(status))
		{
			if (this->_timeout)
			{
				std::cout << "\tCGI TIMEOUT" << std::endl;
				this->setError(504);
			}
			else
			{
				std::cout << "\tCGI ERROR" << std::endl;
				this->setError(502);
			}
			ServerManager::getInstance()->deleteClient(this->_fd[READ]);
			this->_fd[READ] = -1;
			return ;
		}
		while ((read_ret = read(this->_fd[READ], buf, 4)) > 0)
			this->_response += std::string(buf, read_ret);
		if (!is_finished)
		{
			ServerManager::getInstance()->ignoreClient(this->getState()->getSocketFd());
			return;
		}
		ServerManager::getInstance()->deleteClient(this->_fd[READ]);
		this->_fd[READ] = -1;
		if (WEXITSTATUS(status) == ERROR)
		{
			std::cout << "\tCGI ERROR: " << WEXITSTATUS(status) << std::endl;
			return;
		}

		Response	response;

		if (access(_interpreter.c_str(), F_OK) || access(_interpreter.c_str(), X_OK))
		{
			this->setError(502);
			return;
		}

		response.setCode(200);
		response.addHeader("Content-Type", "text/html");
		response.setBody(this->_response);
		this->setResponse(response);
		this->setAsFinished();
	}
}

void CgiStrategy::setTimeout()
{
	errno = 0;
	if (kill(this->_pid, SIGKILL) == -1)
		std::cerr << __FILE__ << ":" << __LINE__ << ": Error: kill(): " << strerror(errno) << std::endl;
	this->_timeout = true;
}

/************************************************************
 *						ENV STUFF							*
 ************************************************************/

void		CgiStrategy::setEnv()
{
	_env["AUTH_TYPE"] = "";
	_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	_env["REDIRECT_STATUS"] = "200";
	_env["SERVER_NAME"] = "webserv";
	_env["METHOD"] = _method == GET ? "GET" : "POST";
	_env["PORT"] = SSTR(this->getState()->getPort());
	_env["SCRIPT_FILENAME"] = this->getState()->getRoot() + _path;
	_env["CONTENT_LENGTH"] = SSTR(this->_cgiBody.length());
	_env["CONTENT_TYPE"] = getContentType(this->getState()->getHeaders());
	if (_method == GET) {
		//this->_cgiBody = "";
		_env["QUERY_STRING"] = _queryString;
	}
	else if (_method == POST) {
		_env["QUERY_STRING"] = this->_cgiBody;
		//this->_cgiBody = _queryString;
	}
};

void	CgiStrategy::convertEnv()
{
	char **envp = new char*[_env.size() + 1];
	int i = 0;
	for (std::map<std::string, std::string>::iterator it = _env.begin(); it != _env.end(); it++)
	{
		std::string tmp = it->first + "=" + it->second;
		envp[i] = new char[tmp.size() + 1];
		strcpy(envp[i], tmp.c_str());
		i++;
	}
 	envp[i] = NULL;
	_envp = envp;
};

void	CgiStrategy::freeEnvp()
{
	for (int i = 0;  _envp[i]; i++)
		delete[] _envp[i];
	delete[] _envp;
};

void	CgiStrategy::setPath()
{
	std::string	path = "";
	path = this->getState()->getPath();
	_queryString = "";
	size_t queryPos = path.find("?");
	if (queryPos != std::string::npos)
	{
		_path = path.substr(0, queryPos);
		_queryString = path.substr(queryPos + 1);
	}
	else
	{
		_path = path;
	}

	this->_scriptName = this->getState()->getRoot() + _path;

	if (access(_scriptName.c_str(), F_OK))
	{
		this->setError(404);
		return;
	}
	if (access(_scriptName.c_str(), R_OK))
	{
		this->setError(403);
	}
};


/************************************************************
 *						EXECUTION							*
 ************************************************************/

int	CgiStrategy::executeScript()
{
	pipe(this->_fd);
	this->_pid = fork();
	if (this->_pid == -1)
		return (1);

	if (this->_pid == 0)
		cgiChildProcess();
	return (0);
};

void	CgiStrategy::cgiChildProcess()
{
	const char *args[] = {_interpreter.c_str(), this->_scriptName.c_str(), NULL};

	for (int i = 0; this->_envp[i]; i++)
		std::cout << this->_envp[i] << std::endl;
	close(this->_fd[READ]);
	this->_fd[READ] = -1;
	dup2(this->_fd[WRITE], STDOUT_FILENO);

	if (this->_cgiBody.length() > 0)
	{
		int _fd2[2];
		pipe(_fd2);
		dup2(_fd2[READ], STDIN_FILENO);
		write(_fd2[WRITE], this->_cgiBody.c_str(), this->_cgiBody.length());
		close(_fd2[WRITE]);
		this->_fd[WRITE] = -1;
	}
	if (execve(args[0], const_cast<char *const *>(args), this->_envp))
	{
		ServerManager::deleteInstance();
		exit(ERROR);
	}
};

/************************************************************
 *						UTILS								*
 ***********************************************************/

int	CgiStrategy::getContentLength(std::map<std::string, std::string> headers) {
	std::map<std::string, std::string>::iterator it = headers.find("Content-Length");
	if (it != headers.end())
		return (atoi(it->second.c_str()));
	return (0);
}

std::string	CgiStrategy::getContentType(std::map<std::string, std::string> headers) {
	std::string contentType = "";
	std::map<std::string, std::string>::iterator it = headers.find("Content-Type");
	if (it != headers.end())
		contentType = it->second;
	return (contentType);
}
