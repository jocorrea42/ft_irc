/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fili <fili@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/21 08:25:16 by fili              #+#    #+#             */
/*   Updated: 2024/09/13 12:44:06 by fili             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client &Client::operator=(Client const &other)
{
	this->_fd = other._fd;
	this->_ipAdd = other._ipAdd;
	this->_nickName = other._nickName;
	this->_user = other._user;
	this->_realname = other._realname;
	this->_inBuffer = other._inBuffer;
	this->_outBuffer = other._outBuffer;
	this->_clientadd = other._clientadd;
	this->_status = other._status;
	std::cout << "Asignacion del Cliente:" << other._ipAdd << ", fd:" << other._fd << std::endl;
	return (*this);
}

void Client::nextStatus()
{
	if (_status == PASS)
		_status = NICK;
	else if (_status == NICK)
		_status = USER;
	else if (_status == USER)
		_status = REG;
}

int Client::sendOwnMessage()
{
	return (send(_fd, _outBuffer.c_str(), _outBuffer.length(), 0) < 0) ? 0 : 1;
}

void Client::removeFirstInCmd()
{
	std::string::size_type index = this->_inBuffer.find(std::string("\r\n"));
	if (index != std::string::npos)
		this->_inBuffer.erase(0, index + 2);
}
void Client::cleanInBuffer()
{
	this->_inBuffer.clear();
}

void Client::cleanOutBuffer()
{
	this->_outBuffer.clear();
}

int Client::receiveMessage()
{
	char buff[MAX_BUFFER_LEN];
	ssize_t bytes;

	bytes = recv(_fd, buff, MAX_BUFFER_LEN - 1, 0);
	if (bytes <= 0)
		return (0);
	buff[bytes] = 0;
	this->addInBuffer(buff);
	return (1);
}

bool Client::msgLon()
{
	std::size_t index = _inBuffer.find(std::string("\r\n"));
	return ((index != std::string::npos) ? (index >= (MAX_BUFFER_LEN - 1)) : (_inBuffer.length() > (MAX_BUFFER_LEN - 2)));
}