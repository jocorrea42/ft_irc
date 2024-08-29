/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fili <fili@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/23 08:12:02 by fili              #+#    #+#             */
/*   Updated: 2024/08/29 09:52:19 by fili             ###   ########.fr       */
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

Channel::Channel(std::string name, Client *client): _name(name), _invOnly(false), _topicLock(false), _limit(0)
{
	_clients.push_back((client->getFd()));
	_admins.push_back(client->getFd());
}

Channel::Channel(std::string name, std::string password, Client *client): _name(name), _password(password), _invOnly(false), _topicLock(false), _limit(0)
{
	_clients.push_back((client->getFd()));
	_admins.push_back(client->getFd());
}

Channel &Channel::operator=(Channel const &other)
{
    this->_name = other._name;
    this->_password = other._password;
    this->_clients = other._clients;
    this->_admins = other._admins;
	_topic = other._topic;
	_invOnly = other._invOnly;
	_topicLock = other._topicLock;
	_invited = other._invited;
	_limit = other._limit;
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

bool Channel::isClient(Client *fd)
{
	for (std::vector<int>::iterator i = _clients.begin(); i != _clients.end(); ++i)
		if ((*i) == fd->getFd())
			return true;
	return false;
}

bool Channel::isInvOnly()
{
	return _invOnly;
}

bool Channel::invite(int fd)
{
	if (fd < 0)
		return false;
	for (std::vector<int>::iterator i = _invited.begin() + 1; i != _invited.end(); ++i)
		if (*i == fd)
			return true;
	_invited.push_back(fd);
	return true;
}

bool Channel::isAdmin(int fd)
{
	for (std::vector<int>::iterator i = _admins.begin(); i != _admins.end(); ++i)
		if (*i == fd)
			return true;
	return false;
}

void Channel::addClient(Client *client)
{
	_clients.push_back(client->getFd());
	client->addOutBuffer(std::string("You joined #" + getName() + " \r\n"));
}

bool Channel::removeClient(int fd)
{
	for (std::vector<int>::iterator i = _clients.begin(); i != _clients.end(); ++i)
	{
		if ((*i) == fd)
		{
			if (isAdmin(fd))
			 	removeAdmin(fd);
			_clients.erase(i);
			std::cout << "se elimino cliente <" << fd << "> del canal: " << _name << std::endl;
			return true;
		}
		
	}
	return false;
}

void Channel::addAdmin(Client *client)
{
	_admins.push_back(client->getFd());
}

void Channel::removeAdmin(int fd)
{
	for (std::vector<int>::iterator i = _admins.begin(); i != _admins.end(); ++i)
		if ((*i) == fd)
		{
			_admins.erase(i);
			break;
		}
}

void Channel::setLimit(int n)
{
	_limit = n;
}

bool Channel::isFull()
{
	if (_limit > 0 && (int)_clients.size() >= _limit)
		return true;
	return false;
}

bool Channel::isTopicLocked()
{
	return _topicLock;
}

std::string Channel::getTopic()
{
	return _topic;
}

void Channel::setTopic(const std::string &newTopic)
{
	_topic = newTopic;
}

void Channel::giveTakeAdmin(int fd, const std::string &nick, Client *client)
{
	for (std::vector<int>::iterator i = _clients.begin(); i != _clients.end(); ++i)
	{
		if ((*i) == fd)
		{
			if (isAdmin(fd))
			{
				client->addOutBuffer(std::string(nick + "'s operator rights removed\r\n"));
				removeAdmin(fd);
			}
			else
			{
				client->addOutBuffer(std::string("operator rights granted to " + nick + "\r\n"));
				_admins.push_back(fd);
			}
			return;
		}
	}
	client->addOutBuffer(std::string(nick + " is not part of this channel\r\n"));
}

std::string			Channel::getMode()
{
	std::string mode = "+";
	if (_invOnly)
		mode += "i";
	if (_topicLock)
		mode += "t";
	if (!getPassword().empty())
		mode += "+k " + getPassword() + " ";
	if (_limit)
	{
		std::stringstream userLimitString;
		userLimitString << _limit;
		mode += "l " + userLimitString.str();
	}
	return (mode);
}	
