/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fili <fili@student.42.fr>                  #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024-05-21 08:18:50 by fili              #+#    #+#             */
/*   Updated: 2024-05-21 08:18:50 by fili             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"

Server::Server() 
{ 
	this->_fd = -1;
	this->_polls_size = 0;
} 
Server::Server(int port, std::string password): _port(port), _pass(password)
{
	this->_fd = -1;
	this->_polls_size = 0;
}

Server::~Server()
{
	for (unsigned short i = 0; i < _polls_size; i++)
		close(_fds[i].fd);
	close(_fd);
	std::cout << "END OF PROGRAM" << std::endl;
};

void Server::ServerSocketCreate()
{
	int opt_val = 1;

	_add.sin_family = AF_INET;		  //-> set the address family to ipv4
	_add.sin_port = htons(this->_port); //-> convert the port to network byte order (big endian)
	_add.sin_addr.s_addr = INADDR_ANY; //-> set the address to any local machine address
	if ((_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) //-> create the server socket and check if the socket is created
		throw(std::runtime_error("faild to create socket"));
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val)) == -1) //-> set the socket option (SO_REUSEADDR) to reuse the address
		throw(std::runtime_error("faild to set option (SO_REUSEADDR) on socket"));
	if (fcntl(_fd, F_SETFL, O_NONBLOCK) == -1) //-> set the socket option (O_NONBLOCK) for non-blocking socket
		throw(std::runtime_error("faild to set option (O_NONBLOCK) on socket"));
	//if (setsockopt(_fd, SOL_SOCKET, SO_NOSIGPIPE, &opt_val, sizeof(opt_val)))
	//	throw std::runtime_error("Failed To Set Socket To Not SIGPIPE");//->// Set SO_NOSIGPIPE option to prevent SIGPIPE signals on write errors
	if (bind(_fd, reinterpret_cast<struct sockaddr*>(&_add), sizeof(_add)) == -1) //-> bind the socket to the address
		throw(std::runtime_error("faild to bind socket"));
	if (listen(_fd, SOMAXCONN) == -1) //-> listen for incoming connections and making the socket a passive socket
		throw(std::runtime_error("listen() faild"));
	addPollfd(_fd);
}

void Server::ServerStart(int port)
{
	this->_port = port;
	unsigned short revents;
	ServerSocketCreate(); //-> create the server socket

	std::cout << "IRC_SERVER <" << _fd << "> is listening!!!!!!!!" << std::endl;
	std::cout << "Wait for clients...\n";
	while (!(this->_Signal))									  //-> run the server until the signal is received
	{																  // poll(fdsarray, fdsarraysize, time) el time en -1 bloquea hasta que exita evento en el poll
		if ((poll(&_fds[0], _polls_size, -1) == -1) && _Signal == false) // codigo para ver si ocurrio un evento
			throw(std::runtime_error("poll() fail, el vento salio mal"));
		for (int i = 0; i < _polls_size; i++) // miro todos los fds en el poll vector
		{
			revents = _fds[i].revents;
			if (revents == 0)
				continue;
			if ((revents & POLLERR) == POLLERR || (revents & POLLHUP) == POLLHUP)
			{
				std::cout << "unexpected client disconnection\n";
				ClearClients(_fds[i].fd);
				break;
			}
			if (revents & POLLIN) // si el and es uno es por que revents es POLLIN o sea hay entrada para ser leida
			{
				if (_fds[i].fd == _fd) // en este caso es una peticion al server de un cliente
					AcceptNewClient();
				else // en este caso es un mensaje de algun cliente
					ReceiveNewData(_fds[i].fd);
			}

		}
	}
	CloseFds();
}

void Server::CloseFds()
{
	for (size_t i = 0; i < _clients.size(); i++)
	{ //-> close all the clients
		std::cout << "Client <" << _clients[i].getFd() << "> Disconnected" << std::endl;
		close(_clients[i].getFd());
	}
	if (_fd != -1)
	{ //-> close the server socket
		std::cout << "Server <" << _fd << "> Disconnected" << std::endl;
		close(_fd);
	}
}

bool Server::_Signal = false;
void Server::SignalHandler(int signum)
{
	(void)signum;
	std::cout << std::endl << "Signal Received!" << std::endl;
	_Signal = true; //-> set the static boolean to true to stop the server
}

void Server::AcceptNewClient()//agregamos un  cliente a la lista de clientes
{
	int inConectionFd;
	Client cli; //-> create a new client
	socklen_t len = sizeof(_clientadd);

	if ((inConectionFd = accept(_fd, (sockaddr *)&(_clientadd), &len)) == -1) //-> accept the new client
	{
		std::cout << "accept() failed" << std::endl;
		return;
	}

	if (fcntl(inConectionFd, F_SETFL, O_NONBLOCK) == -1) //-> set the socket option (O_NONBLOCK) for non-blocking socket
	{
		std::cout << "fcntl() failed" << std::endl;
		return;
	}
	cli.setFd(inConectionFd);						 //-> set the client file descriptor
	cli.setIp(inet_ntoa((_clientadd.sin_addr))); //-> convert the ip address to string and set it
	_clients.push_back(cli);					 //-> add the client to the vector of clients
    addPollfd(inConectionFd);// -> agrega un nuevo fd a la lista de poll para la escucha de un evento
	std::cout << "Client <" << inConectionFd << "> Connected" << std::endl;
}

void Server::ReceiveNewData(int fd)
{
	char buff[1024]; //-> buffer for the received data
	memset(buff, 0, sizeof(buff)); //-> clear the buffer

	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1, 0); //-> receive the data

	if (bytes <= 0)
	{ //-> check if the client disconnected
		std::cout << "Client <" << fd << "> Disconnected" << std::endl;
		ClearClients(fd); //-> clear the client
		close(fd);		  //-> close the client socket
	}

	else
	{ //-> print the received data
		buff[bytes] = '\0';
		std::cout << "Client <" << fd << "> Data: " << buff;
		send(fd, buff, 0, sizeof(buff));
		// here you can add your code to process the received data: parse, check, authenticate, handle the command, etc...
	}
}

void Server::ClearClients(int fd)
{ //-> clear the clients
	for (size_t i = 0; i < _fds.size(); i++)
	{ //-> remove the client from the pollfd
		if (_fds[i].fd == fd)
		{
			_fds.erase(_fds.begin() + i);
			_polls_size--;
			break;
		}
	}
	for (size_t i = 0; i < _clients.size(); i++)
	{ //-> remove the client from the vector of clients
		if (_clients[i].getFd() == fd)
		{
			_clients.erase(_clients.begin() + i);
			break;
		}
	}
}

void		Server::addPollfd(int fd)
{
	struct pollfd newFdClientPoll;	  // lo mismo, estas tres estructuras se utilizan para la creacion y control de
								  // los nuevos clientes conectados
	memset(&newFdClientPoll, 0, sizeof(newFdClientPoll));
	newFdClientPoll.fd = fd; //-> add the server socket to the pollfd
	newFdClientPoll.events = POLLIN;  //-> set the event to POLLIN for reading data
	//newFdClientPoll.revents = 0;	  //-> set the revents to 0
	_fds.push_back(newFdClientPoll);	  //-> add the server socket to the pollfd
	_polls_size++;
}

void	Server::setFd(int fd)
{
	this->_fd = fd;
}
void	Server::setPassword(std::string pass)
{
	this->_pass = pass;
}

void	Server::setPort(int port)
{
	this->_port = port;
}

int	Server::getPort()
{
	return (this->_port);
}

int	Server::getFd()
{
	return (this->_fd);
}

Client	*Server::getClient(int fd)
{
	for (size_t i = 0; i < _clients.size(); i++)
	{ //-> encuentra el cliente con ese fd
		if (_clients[i].getFd() == fd)
			return (&(this->_clients[i]));
	}
	return (NULL);
}

Client *Server::getClientNick(std::string nickname){
	for (size_t i = 0; i < this->_clients.size(); i++){
		if (this->_clients[i].getNickName() == nickname)
			return &this->_clients[i];
	}
	return NULL;
}

void Server::RemoveChannel(std::string name){
	for (size_t i = 0; i < this->_channels.size(); i++){
		if (this->_channels[i].GetName() == name)
		{
			this->_channels.erase(this->_channels.begin() + i); 
			break;
		}
	}
}