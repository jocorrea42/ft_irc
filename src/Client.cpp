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
    this->_ip_add = other._ip_add;
    this->_nickName = other._nickName;
    std::cout << "se ha asignado un cliente:" << other._ip_add << ", fd:" << other._fd << std::endl;
    return (*this);
} 