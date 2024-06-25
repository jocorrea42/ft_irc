/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Clien.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fili <fili@student.42.fr>                  #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024-05-21 08:25:16 by fili              #+#    #+#             */
/*   Updated: 2024-05-21 08:25:16 by fili             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Client.hpp"

Client::Client(Client const &other){*this = other;}
Client  &Client::operator=(Client const &other)
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

void    Client::sendMessage(std::string sms)
{

   int bytes = send(_fd, sms.c_str(), sms.length(), 0);

}

void Client::cleanBuffer()
{
    std::cout << "limpiando bufer: " << this->_outBuffer << std::endl;
    _outBuffer.clear();
	//std::string::size_type index = this->_outBuffer.find(std::string("\r\n"));
	//if (index != std::string::npos)
	//	this->_outBuffer.erase(0, index);
    std::cout << "limpio bufer: " << this->_outBuffer << std::endl;
}