/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jocorrea <jocorrea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/28 13:28:57 by jocorrea          #+#    #+#             */
/*   Updated: 2024/09/22 17:00:00 by jocorrea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Client *Server::getClient(const std::string &nick)
{
	for (size_t i = 0; i < _clients.size(); i++)
		if (_clients[i].getNickName() == nick)
			return &_clients[i];
	return (NULL);
}

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
	{
		close(_clients[i].getFd());
		_clients.erase(_clients.begin() + i);
	}
	if (_fd != -1)
		close(_fd);
	_clients.clear();
}

bool Server::_Signal = false;
void Server::SignalHandler(int signum)
{
	(void)signum;
	_Signal = true;
}

Server::Server()
{
	this->_fd = -1;
	this->_polls_size = 0;
	_add.sin_family = AF_INET;
	_add.sin_addr.s_addr = INADDR_ANY;
}

Server::Server(int port, std::string password) : _port(port), _pass(password)
{
	this->_fd = -1;
	this->_polls_size = 0;
	_add.sin_family = AF_INET;
	_add.sin_addr.s_addr = INADDR_ANY;
	_add.sin_port = htons(this->_port);
	_channels.push_back(Channel("Bot"));
}

Server::~Server()
{
	for (unsigned short i = 0; i < _polls_size; i++)
		close(_fds[i].fd);
	close(_fd);
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

void Server::_ClearClient(int fd)
{
	for (size_t i = 0; i < _fds.size(); i++)
		if (_fds[i].fd == fd)
		{
			_fds.erase(_fds.begin() + i);
			_polls_size--;
			close(fd);
			break;
		}
	for (size_t i = 0; i < _clients.size(); i++)
		if (_clients[i].getFd() == fd)
		{
			_clients.erase(_clients.begin() + i);
			break;
		}
}

void Server::addPollfd(int fd)
{
	struct pollfd newFdClientPoll;
	memset(&newFdClientPoll, 0, sizeof(newFdClientPoll));
	newFdClientPoll.fd = fd;
	newFdClientPoll.events = POLLIN;
	_fds.push_back(newFdClientPoll);
	_polls_size++;
}

void Server::_broadcastClientChannel(Channel *channel, std::string msg, int fd)
{
	std::vector<std::string> clients = channel->getClients();
	clients = channel->getClients();
	Client *client;
	for (size_t j = 0; j < clients.size(); j++)
	{
		client = getClient(clients[j]);
		if (client && client->getFd() != fd)
			client->addOutBuffer(msg);
	}
}

void Server::_disconnectClient(Client *client, std::string msg, int mode)
{
	std::string quit_msg = ":" + client->getNickName() + "!~" + client->getName() + " QUIT :" + msg + " \r\n";
	if (mode == 1)
		_broadcastAllServer(quit_msg);
	for (size_t i = 0; i < _channels.size(); i++)
	{
		if (_channels[i].isClient(client))
		{
			_channels[i].removeClient(client->getNickName());
			if (_channels[i].getClients().size() == 0)
				_channels.erase(_channels.begin() + i);
			else if (_channels[i].hasAdmin() == 0)
			{
				Client *newAdmin = getClientNick(_channels[i].getClients()[0]);
				_channels[i].addAdmin(newAdmin->getNickName());
				newAdmin->addOutBuffer(std::string(_channels[i].getName() + " No admins left connected/you are the new Admin \r\n"));
			}
		}
	}
	_ClearClient(client->getFd());
}

std::vector<std::string> Server::_splitStr(const std::string &str, char delimiter)
{
	std::vector<std::string> tokens;
	std::istringstream iss(str);
	std::string token;
	while (std::getline(iss, token, delimiter))
		tokens.push_back(token);
	return tokens;
}

void Server::changeNickname(Client *client, const std::string &newNick)
{
	for (std::vector<Channel>::iterator i = _channels.begin(); i != _channels.end(); ++i)
	{
		if (i->isClient(client))
		{
			if (i->isAdmin(client->getNickName()))
			{
				i->removeAdmin(client->getNickName());
				i->addAdmin(newNick);
			}
			i->removeClient(client->getNickName());
			i->addClient(newNick);
		}
	}
	client->setNickName(newNick);
}