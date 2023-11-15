/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerResponse.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vlepille <vlepille@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/01 15:53:57 by chmadran          #+#    #+#             */
/*   Updated: 2023/11/15 21:46:23 by fguarrac         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerResponse.hpp"
#include "ServerManager.hpp"
#include "Route.hpp"
#include <sstream>

ServerResponse::ServerResponse(): _autoindex(false), _error_code(0), _method(0), _redirect_type(0)
{}

void ServerResponse::prepare(const ClientRequest &request)
{
	if (!request.server)
	{
		this->_error_code = 500;
		return ;
	}

	const Route	*route = request.server->getRoute(request.getPath());

	// ## Request ##
	// Client socket
	this->_client_socket = request.getClientSocket();
	// Error pages
	this->_error_pages = request.server->getErrorPages();
	if (request.getErrorCode())
	{
		this->_error_code = request.getErrorCode();
		return ;
	}
	// Method
	if (route && route->hasMethods())
		this->_method = route->getMethods() & request.getMethod();
	else
		this->_method = request.server->getMethods() & request.getMethod();
	if (!this->_method)
	{
		this->_error_code = 405;
		return ;
	}
	if (request.protocol != HTTP_PROTOCOL)
	{
		this->_error_code = 505;
		return ;
	}
	this->_path = request.getPath();
	this->_headers = request.getHeaders();

	// ## Server ##
	// Autoindex
	if (route && route->hasAutoindex())
		this->_autoindex = route->getAutoindex();
	else
		this->_autoindex = request.server->getAutoindex();
	// Root
	if (route && route->hasRoot())
		this->_root = route->getRoot();
	else
		this->_root = request.server->getRoot();
	// Index
	if (route && route->hasIndex())
		this->_index = route->getIndex();
	else
		this->_index = request.server->getIndex();
	if (route)
	{
		// Redirect
		if (route->hasRedirect())
		{
			this->_redirect_type = route->getRedirectType();
			this->_redirect = route->getRedirect();
		}
		// CGI
		if (route->hasCgi())
		{
			this->_cgi_extension = route->getCgiExtension();
			this->_cgi_path = route->getCgiPath();
		}
		// Upload
		if (route->hasUpload())
			this->_upload_path = route->getUploadPath();
	}
}

//std::string		ServerResponse::_getGenericErrorPage(void) const
//{
//	std::map<int, std::string> const		error_codes = {
//		std::make_pair(400, "Bad Request"),
//		std::make_pair(401, "Unauthorized"),
//		std::make_pair(402, "Payment Required"),
//		std::make_pair(403, "Forbidden"),
//		std::make_pair(404, "Not Found"),
//		std::make_pair(405, "Method Not Allowed"),
//		std::make_pair(406, "Not Acceptable"),
//		std::make_pair(407, "Proxy Authentication Required"),
//		std::make_pair(408, "Request Time-out"),
//		std::make_pair(409, "Conflict"),
//		std::make_pair(410, "Gone"),
//		std::make_pair(411, "Length Required"),
//		std::make_pair(412, "Precondition Failed"),
//		std::make_pair(413, "Request Entity Too Large"),
//		std::make_pair(414, "Request-URI Too Long"),
//		std::make_pair(415, "Unsupported Media Type"),
//		std::make_pair(416, "Requested range unsatisfiable"),
//		std::make_pair(417, "Expectation failed"),
//		std::make_pair(418, "I’m a teapot"),
//		std::make_pair(419, "Page expired"),
//		std::make_pair(421, "Bad mapping / Misdirected Request"),
//		std::make_pair(422, "Unprocessable entity"),
//		std::make_pair(423, "Locked"),
//		std::make_pair(424, "Method failure"),
//		std::make_pair(425, "Too Early"),
//		std::make_pair(426, "Upgrade Required"),
//		std::make_pair(427, "Invalid digital signature"),
//		std::make_pair(428, "Precondition Required"),
//		std::make_pair(429, "Too Many Requests"),
//		std::make_pair(431, "Request Header Fields Too Large"),
//		std::make_pair(449, "Retry With"),
//		std::make_pair(450, "Blocked by Windows Parental Controls"),
//		std::make_pair(451, "Unavailable For Legal Reasons"),
//		std::make_pair(456, "Unrecoverable Error"),
//		std::make_pair(444, "No Response"),
//		std::make_pair(495, "SSL Certificate Error"),
//		std::make_pair(496, "SSL Certificate Required"),
//		std::make_pair(497, "HTTP Request Sent to HTTPS Port"),
//		std::make_pair(498, "Token expired/invalid"),
//		std::make_pair(499, "Client Closed Request"),
//		std::make_pair(500, "Internal Server Error"),
//		std::make_pair(501, "Not Implemented"),
//		std::make_pair(502, "Bad Gateway ou Proxy Error"),
//		std::make_pair(503, "Service Unavailable"),
//		std::make_pair(504, "Gateway Time-out"),
//		std::make_pair(505, "HTTP Version not supported"),
//		std::make_pair(506, "Variant Also Negotiates"),
//		std::make_pair(507, "Insufficient storage"),
//		std::make_pair(508, "Loop detected"),
//		std::make_pair(509, "Bandwidth Limit Exceeded"),
//		std::make_pair(510, "Not extended"),
//		std::make_pair(511, "Network authentication required"),
//		std::make_pair(520, "Unknown Error"),
//		std::make_pair(521, "Web Server Is Down"),
//		std::make_pair(522, "Connection Timed Out"),
//		std::make_pair(523, "Origin Is Unreachable"),
//		std::make_pair(524, "A Timeout Occurred"),
//		std::make_pair(525, "SSL Handshake Failed"),
//		std::make_pair(526, "Invalid SSL Certificate"),
//		std::make_pair(527, "Railgun Error")
//	};
//	std::string const				generic_page = "<!DOCTYPE html>\n<html lang=\"en\">\n\t<head>\n\t\t<meta charset=\"UTF-8\">\n\t\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n\t\t<title>{code} {message}</title>\n\t</head>\n\t<body>\n\t\t<center><h1>{code} {message}</h1></center>\n\t\t<hr><center>webserv</center>\n\t</body>\n</html>";
//
//	size_t	index;
//
//	index = generic_page.find("{code}");
//	generic_page.replace(index, 6, this->_error_code);
//	index = generic_page.find("{message}");
//	generic_page.replace(index, 9, error_codes[this->_error_code]);
//	return (generic_page);
//}

void ServerResponse::process()
{
	std::string content;
	if (this->_error_code)
	{
//		if ((std::map<int, std::string>::iterator it = this->_error_pages.find(this->_error_code)) != this->_error_pages.end())
//			content = readFileContent(this->_error_pages[this->_error_code]);
//		else
//			content = _getGenericErrorPage();
//		sendHttpResponse(this->_client_socket, content, "text/html");	//	@TODO	send proper status code
//		return;
	}
	if (this->_method == GET) {
		if (this->_path == "/")
		{
			content = readFileContent("src/" + this->_index);
			sendHttpResponse(this->_client_socket, content, "text/html");
		}
		else
		{
			content = readFileContent("src/" + this->_path);
			sendHttpResponse(this->_client_socket, content, "text/" + this->_path.substr(this->_path.find_last_of(".") + 1));
		}
	} else if (this->_method == POST) {
		// Handle POST request
		// Generate appropriate response
	} else if (this->_method == DELETE) {
		// Handle DELETE request
		// Generate appropriate response
	}
	//ADD ERROR HANDLING HERE
}

std::string ServerResponse::readFileContent(const std::string& filePath) {
	std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);
	if (!file.is_open()) {
		perror("In opening file"); // @TODO return 404
		//exit(EXIT_FAILURE); // @TODO return 404
	}
	return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

void ServerResponse::sendHttpResponse(int clientSocket, const std::string& content, const std::string& contentType) {
	std::stringstream httpHeaders;
	httpHeaders << "HTTP/1.1 200 OK\r\n" <<
				"Content-Length: " << content.size() << "\r\n" <<
				"Content-Type: " << contentType << "\r\n" <<
				"\r\n";
	std::string httpResponse = httpHeaders.str() + content;
	write(clientSocket, httpResponse.c_str(), httpResponse.size());
}
