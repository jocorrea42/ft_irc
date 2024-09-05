/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jocorrea <jocorrea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/23 08:04:59 by fili              #+#    #+#             */
/*   Updated: 2024/09/05 15:21:27 by jocorrea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP
# include "Server.hpp"
# define MAX_BUFFER_LEN 512

enum Status { PASS, NICK, USER, REG };

class Client
{
private:
	//client tecnical data
	Status			_status;
	int 			_fd;
	struct sockaddr_in _clientadd; 
	std::string 	_ipAdd;
	//user setting
	std::string 	_user;
	std::string 	_realname;
	std::string		_nickName;
	// client messages
	std::string		_outBuffer;
	std::string		_inBuffer;

public:
	Client(): _status(PASS){std::cout << "se construyo un nuevo cliente vacio\n";}
	Client(int fd, sockaddr_in addr) :_status(PASS), _fd(fd),  _clientadd (addr), _nickName("Cliente") {this->_ipAdd = inet_ntoa(_clientadd.sin_addr); std::cout << "nuevo cliente con ip:" << _ipAdd << ", fd:" << _fd << std::endl;}
	Client(Client const &other){*this = other;}
	~Client(){std::cout << "se destruyo el cliente " << _nickName << " fd <" << _fd << "> " << std::endl;}
	Client &operator=(Client const &other);

	void		setIp(std::string ip) { this->_ipAdd = ip; }
	void		setFd(int fd) { this->_fd = fd; }
	void		setNickName(std::string	nick){ this->_nickName = nick;}
	void		setOutBuffer(std::string outbuffer){ this->_outBuffer = outbuffer;}
	void		setInBuffer(std::string inbuffer){ this->_inBuffer = inbuffer;}
	void		setUser(std::string user){ this->_user = user;}
	void		setName(std::string name){ this->_realname = name;}
	void		setStatus(Status status){ this->_status = status;}

	int			getFd(){ return this->_fd; }
	Status		getStatus(){ return (this->_status);}
	std::string getName(){ return (this->_realname);}
	std::string	getUser(){ return (this->_user);}
	std::string getOutBuffer(){ return (this->_outBuffer);}
	std::string getInBuffer(){ return (this->_inBuffer);}
	std::string	getNickName(){ return (this->_nickName);}
	std::string getIp(){ return this->_ipAdd; }

	bool		msgLon();
	void		nextStatus();
	void		cleanInBuffer();
	void		cleanOutBuffer();
	void		removeFirstInCmd();
	void		addOutBuffer(char *msg){ this->_outBuffer += std::string(msg);}
	void		addOutBuffer(const std::string &msg){ this->_outBuffer += msg;}
	void		addInBuffer(char *msg){ this->_inBuffer += std::string(msg);}
	int			sendOwnMessage();//el cliente envia el mensaje conformado por el server almacenado en su buffer de salida
	int			receiveMessage();//el cliente recibe un mensaje desde su fd y lo almacena en su buffer de entrada
};
#endif