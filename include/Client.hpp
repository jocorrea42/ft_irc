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


class Client
{
private:
	int _fd;
	std::string _ip_add;
	std::string	_nickName;

public:
	Client(){std::cout << "se construyo un nuevo cliente vacio\n";}
	Client(std::string ip_add) : _ip_add(ip_add) {std::cout << "nuevo cliente con ip:" << _ip_add << std::endl;}
	Client(int fd) : _fd(fd) {std::cout << "nuevo cliente con fd:" << _fd << std::endl;}
	Client(int fd, std::string ipadd) : _fd(fd), _ip_add(ipadd) {std::cout << "nuevo cliente con ip:" << _ip_add << ", fd:" << _fd << std::endl;}
	Client(Client const &other);
	~Client(){std::cout << "se destruyo el cliente " << _nickName << std::endl;}
	Client &operator=(Client const &other);
	void	setIp(std::string ip) { this->_ip_add = ip; }
	void	setFd(int fd) { this->_fd = fd; }
	void	setNickName(std::string	nick){this->_nickName = nick;}
	int		getFd() { return this->_fd; }
	std::string	getNickName(){return (this->_nickName);}
	std::string getIp() { return this->_ip_add; }
};
#endif