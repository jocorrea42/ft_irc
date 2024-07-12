/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apodader <apodader@student.42barcel>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/23 08:12:02 by fili              #+#    #+#             */
/*   Updated: 2024/07/12 20:25:13 by apodader         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

// Channel();
//     ~Channel();
//     Channel(Channel const &src);
//     Channel &operator=(Channel const &src);
//     void SetPassword(std::string password);
//     void SetName(std::string name);
//     std::string GetPassword();
// 	std::string GetName();
//     void add_client(Client newClient);
//     void add_admin(Client newClient);
//     void remove_client(int fd);
//     void remove_admin(int fd);
//     bool change_clientToAdmin(std::string &nick);
//     bool change_adminToClient(std::string &nick);
// 	void sendTo_all(std::string rpl1, int fd);
Channel::Channel(){}

Channel::~Channel(){}

Channel::Channel(std::string name, Client *client): _name(name), _invOnly(false)
{
	_clients.push_back(client);
	_admins.push_back(client->getNickName());
}

Channel::Channel(std::string name, std::string password, Client *client): _name(name), _password(password), _invOnly(false)
{
	_clients.push_back(client);
	_admins.push_back(client->getNickName());
}

Channel &Channel::operator=(Channel const &other)
{
    this->_name = other._name;
    this->_password = other._password;
    this->_clients = other._clients;
    this->_admins = other._admins;
    return (*this);
}

bool Channel::isInvited(int fd)
{
	for (std::vector<int>::iterator i = _invited.begin(); i != _invited.end(); ++i)
	{
		if (*i == fd)
		{
			_invited.erase(i);
			return true;
		}
	}
	return false;
}

bool Channel::getClient(std::string nick)
{
	for (std::vector<Client*>::iterator i = _clients.begin(); i != _clients.end(); ++i)
		if ((*i)->getNickName() == nick)
			return true;
	return false;
}

bool Channel::isInvOnly()
{
	return _invOnly;
}

bool Channel::isAdmin(const std::string &nickname)
{
	for (std::vector<std::string>::iterator i = _admins.begin(); i != _admins.end(); ++i)
		if (*i == nickname)
			return true;
	return false;
}

void Channel::add_client(Client *client)
{
	_clients.push_back(client);
	client->addOutBuffer(std::string("You joined #" + GetName() + " \r\n"));
}

bool Channel::remove_client(const std::string &nick)
{
	for (std::vector<Client*>::iterator i = _clients.begin(); i != _clients.end(); ++i)
	{
		if ((*i)->getNickName() == nick)
		{
			if (isAdmin(nick))
				remove_admin(nick);
			_clients.erase(i);
			return true;
		}
	}
	return false;
}

void Channel::add_admin(Client *client)
{
	_admins.push_back(client->getNickName());
}

void Channel::remove_admin(const std::string &nick)
{
	for (std::vector<std::string>::iterator i = _admins.begin(); i != _admins.end(); ++i)
		if (*i == nick)
			_admins.erase(i);
}

void Channel::sendToAll(std::string msg)
{
	for (std::vector<Client*>::iterator i = _clients.begin(); i != _clients.end(); ++i)
		(*i)->addOutBuffer(msg + "\r\n");
}
