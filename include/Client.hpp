/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apodader <apodader@student.42barcel>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/23 08:04:59 by fili              #+#    #+#             */
/*   Updated: 2024/06/26 02:05:44 by apodader         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP
# include "Server.hpp"

enum Status { PASS, NICK, USER, REG };

class Client
{
private:
	//client tecnical data
	Status			_status;
	int 			_fd;
	std::string 	_ipAdd;
	//user setting
	std::string 	_user;
	std::string 	_realname;
	std::string		_nickName;
	// client messages
	std::string		_outBuffer;
	//channels
	

public:
	Client(): _status(PASS){std::cout << "se construyo un nuevo cliente vacio\n";}
	Client(std::string ip_add) :_status(PASS), _ipAdd(ip_add) {std::cout << "nuevo cliente con ip:" << _ipAdd << std::endl;}
	Client(int fd) : _fd(fd) {std::cout << "nuevo cliente con fd:" << _fd << std::endl;}
	Client(int fd, std::string ipadd) :_status(PASS), _fd(fd), _ipAdd(ipadd) {std::cout << "nuevo cliente con ip:" << _ipAdd << ", fd:" << _fd << std::endl;}
	Client(Client const &other);
	~Client(){std::cout << "se destruyo el cliente " << _nickName << std::endl;}
	Client &operator=(Client const &other);

	void	setIp(std::string ip) { this->_ipAdd = ip; }
	void	setFd(int fd) { this->_fd = fd; }
	void	setNickName(std::string	nick){this->_nickName = nick;}
	void	setBuffer(std::string buffer){this->_outBuffer = buffer;}
	void	setUser(std::string user){this->_user = user;}
	void	setName(std::string name){this->_realname = name;}
	void	setStatus(Status status){this->_status = status;}
	void	nextStatus();
	void	cleanBuffer();
	Status			getStatus(){return (this->_status);}
	std::string getName(){return (this->_realname);}
	std::string	getUser(){return (this->_user);}
	std::string getBuffer(){return (this->_outBuffer);}
	int		getFd() { return this->_fd; }
	std::string	getNickName(){return (this->_nickName);}
	std::string getIp() { return this->_ipAdd; }
	void	addBuffer(char *buff){ this->_outBuffer += std::string(buff);}
	void	sendMessage(std::string sms);
};
#endif