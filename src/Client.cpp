/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jocorrea <jocorrea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/21 08:25:16 by fili              #+#    #+#             */
/*   Updated: 2024/07/28 16:34:38 by jocorrea         ###   ########.fr       */
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

int    Client::sendOwnMessage()
{
	
	int ret = send(_fd, NULL, 0, 0);
	if (ret == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
	{
		// The writing operation would block
		std::cout << "mandando a:" << this->_nickName << " msg: " << this->_outBuffer << std::endl;
		return 0;
	}
	else if (ret == -1)
	{
		// -1 == disconected client
		return 0;
	}
	
	int bytes = send(_fd, _outBuffer.c_str(), _outBuffer.length(), 0);
	if (bytes == -1 && errno != EAGAIN && errno != EWOULDBLOCK)// -1 == disconected client
			return 0;///mal
	//_outBuffer.clear();
	return (1);//bien
}

void Client::removeFirstInCmd()
{ 
	std::string::size_type index = this->_inBuffer.find(std::string("\r\n"));
	if (index != std::string::npos)
		this->_inBuffer.erase(0, index + 2);
}
void	Client::cleanInBuffer()
{
	this->_inBuffer.clear();
}

void	Client::cleanOutBuffer()
{
	this->_outBuffer.clear();
}

int		Client::receiveMessage()
{
	char buff[1024];
	ssize_t bytes;

	while (1)
	{   //-> buffer for the received data
		memset(&buff, 0, sizeof(buff));
		bytes = recv(_fd, buff, sizeof(buff) - 1, 0); //-> receive the data
		if (bytes <= 0)
		{ //-> check if the client disconnected
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				break;
			else
			{
				std::cout << "Client este que esta aqui <" << _fd << "> Disconnected" << std::endl;
				return 0;
			}
		}
		buff[bytes] = 0;
		this->addInBuffer(buff);
	}
	
	return (1);
}
