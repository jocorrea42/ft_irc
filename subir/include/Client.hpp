/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fili <fili@student.42.fr>                  #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024-05-23 08:04:59 by fili              #+#    #+#             */
/*   Updated: 2024-05-23 08:04:59 by fili             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP
# include "Server.hpp"
# include "Channel.hpp"

class Client
{
private:
	int fd;
	std::string ip_add;

public:
	Client(){};
	Client(std::string ip_add) : ip_add(ip_add) {}
	Client(int fd) : fd(fd) {}
	Client(int fd, std::string ipadd) : fd(fd), ip_add(ipadd) {}
	//Client(Client const &other);
	~Client(){}
	//Client &operator=(Client const &other);
	void setIp(std::string ip) { this->ip_add = ip; }
	void setFd(int fd) { this->fd = fd; }
	int getFd() { return this->fd; }
	std::string getIp() { return this->ip_add; }
};
#endif