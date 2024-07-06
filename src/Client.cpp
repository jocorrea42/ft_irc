/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apodader <apodader@student.42barcel>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/21 08:25:16 by fili              #+#    #+#             */
/*   Updated: 2024/06/26 02:06:43 by apodader         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client &Client::operator=(Client const &other)
{
	this->_fd = other._fd;
	this->_ipAdd = other._ipAdd;
	this->_nickName = other._nickName;
	std::cout << "se ha asignado un cliente:" << other._ipAdd << ", fd:" << other._fd << std::endl;
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
	int bytes = send(_fd, _outBuffer.c_str(), _outBuffer.length(), 0);
	if (bytes == -1)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
		{
			// -1 == disconected client
			return 0;
		}
	}
	return (1);
}

void Client::cleanInBuffer()
{
    std::cout << "limpiando  primer comando de inbufer: " << this->_inBuffer << std::endl;
   // _inBuffer.clear();
	std::string::size_type index = this->_inBuffer.find(std::string("\r\n"));
	if (index != std::string::npos)
		this->_inBuffer.erase(0, index + 2);
	std::cout << "el indice es: " << index << ", nuevo inbufer: " << this->_inBuffer << std::endl;
}

int		Client::receiveMessage()
{
	while (1)
	{
		char buff[1024] = {0}; //-> buffer for the received data

		ssize_t bytes = recv(_fd, buff, sizeof(buff) - 1, 0); //-> receive the data
		if (bytes <= 0)
		{ //-> check if the client disconnected
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				break;
			else
			{
				std::cout << "Client <" << _fd << "> Disconnected" << std::endl;
				return 0;
			}
		}
		this->addInBuffer(buff);
	}
	return (1);
}
