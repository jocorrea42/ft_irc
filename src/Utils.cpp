/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fili <fili@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/28 13:28:57 by jocorrea          #+#    #+#             */
/*   Updated: 2024/08/29 09:54:24 by fili             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Client *Server::getClient(const int &fd)
{
	for (size_t i = 0; i < _clients.size(); i++)
		if (_clients[i].getFd() == fd)
			return &_clients[i];
	return (NULL);
}

Client *Server::getClientNick(const std::string &nickname)
{
	for (std::vector<Client>::iterator i = _clients.begin(); i != _clients.end(); ++i)
		if ((*i).getNickName() == nickname)
			return &(*i);
	return NULL;
}

int Server::getClientFd(const std::string &nick)
{
	for (std::vector<Client>::iterator i = _clients.begin(); i != _clients.end(); ++i)
		if ((*i).getNickName() == nick)
			return (*i).getFd();
	return -1;
}

Channel *Server::getChannel(const std::string &name)
{
	for (std::vector<Channel>::iterator i = _channels.begin(); i != _channels.end(); ++i)
		if (i->getName() == name)
			return &(*i);
	return NULL;
}

void Server::RemoveChannel(const std::string &name)
{
	for (std::vector<Channel>::iterator i = _channels.begin(); i != _channels.end(); ++i)
		if (i->getName() == name)
		{
			_channels.erase(i);
			break;
		}
}

Server &Server::operator=(Server const &other)
{
	this->_add = other._add;
	this->_channels = other._channels;
	this->_clients = other._clients;
	this->_fd = other._fd;
	this->_fds = other._fds;
	this->_pass = other._pass;
	this->_polls_size = other._polls_size;
	this->_port = other._port;
	this->_Signal = other._Signal;
	return (*this);
}

void Server::_CloseFds()
{
	for (size_t i = 0; i < _clients.size(); i++)
	{ //-> close all the clients
		std::cout << "Client <" << _clients[i].getFd() << "> Disconnected" << std::endl;
		close(_clients[i].getFd());
		_clients.erase(_clients.begin() + i);
	}
	if (_fd != -1)
	{ //-> close the server socket
		std::cout << "Server <" << _fd << "> Disconnected" << std::endl;
		close(_fd);
	}
	_clients.clear();
}

bool Server::_Signal = false;
void Server::SignalHandler(int signum)
{
	(void)signum;	// std::cout << std::endl << "Signal Received!" << std::endl;
	_Signal = true; //-> set the static boolean to true to stop the server
}

Server::Server()
{
	this->_fd = -1;
	this->_polls_size = 0;
	_add.sin_family = AF_INET;		   //-> set the address family to ipv4
	_add.sin_addr.s_addr = INADDR_ANY; //-> set the address to any local machine address
}

Server::Server(int port, std::string password) : _port(port), _pass(password)
{
	this->_fd = -1;
	this->_polls_size = 0;
	_add.sin_family = AF_INET;			//-> set the address family to ipv4
	_add.sin_addr.s_addr = INADDR_ANY;	//-> set the address to any local machine address
	_add.sin_port = htons(this->_port); //-> convert the port to network byte order (big endian)
}

Server::~Server()
{
	for (unsigned short i = 0; i < _polls_size; i++)
		close(_fds[i].fd);
	close(_fd);
	std::cout << "server destroy!!" << std::endl;
}

void Server::setFd(const int &fd)
{
	this->_fd = fd;
}
void Server::setPassword(const std::string &pass)
{
	this->_pass = pass;
}

void Server::setPort(const int &port)
{
	this->_port = port;
}

int const &Server::getPort()
{
	return (this->_port);
}

int const &Server::getFd()
{
	return (this->_fd);
}

void Server::printParam(std::vector<std::string> params)
{
	std::cout << "PARAMS" << std::endl;
	for (size_t i = 0; i < params.size(); ++i)
		std::cout << "|" << params[i] << "|" << std::endl;
	std::cout << "END" << std::endl;
}

void Server::_ClearClient(int fd)
{											 //-> clear the clients
	for (size_t i = 0; i < _fds.size(); i++) //-> remove the client from the pollfd
		if (_fds[i].fd == fd)
		{
			_fds.erase(_fds.begin() + i);
			_polls_size--;
			close(fd);
			break;
		}
	for (size_t i = 0; i < _clients.size(); i++) //-> remove the client from the vector of clients
		if (_clients[i].getFd() == fd)
		{
			_clients.erase(_clients.begin() + i);
			break;
		}
}

void Server::addPollfd(int fd)
{
	struct pollfd newFdClientPoll; // lo mismo, estas tres estructuras se utilizan para la creacion y control delos nuevos clientes conectados
	memset(&newFdClientPoll, 0, sizeof(newFdClientPoll));
	newFdClientPoll.fd = fd;		 //-> add the server socket to the pollfd
	newFdClientPoll.events = POLLIN; //-> set the event to POLLIN for reading data
	_fds.push_back(newFdClientPoll); //-> add the server socket to the pollfd
	_polls_size++;
}

void	Server::_broadcastClientChannel(Channel *channel, std::string msg, int fd)
{
	std::vector<int> clients = channel->getClients();

			clients = channel->getClients();
			Client *client;
			for (size_t j = 0; j < clients.size(); j++)
			{
				client = getClient(clients[j]);
				if (client && client->getFd() != fd)
					client->addOutBuffer(msg);
			}
}


void Server::_disconnectClient(Client* client, std::string msg)
{
	std::cout << "unexpected dc" << std::endl;
	// Send a quit message to all channels the client is in + remove client from all channels it is in
	std::string quit_msg = ":" + client->getNickName() + "!~" + client->getName() + " QUIT :" + msg + " \r\n";
	_broadcastAllServer(quit_msg);
	// remove client out of all joined channels
	for (size_t i = 0; i < _channels.size(); i++)
		if (_channels[i].isClient(client))
			_channels[i].removeClient(client->getFd());
	_ClearClient(client->getFd());
}