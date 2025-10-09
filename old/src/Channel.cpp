/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apodader <apodader@student.42barcel>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/23 08:12:02 by fili              #+#    #+#             */
/*   Updated: 2024/09/11 18:20:50 by apodader         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"

Channel::Channel(){}

Channel::~Channel(){}
Channel::Channel(std::string name): _name(name)
{
	_clients.push_back("Bot");
	_admins.push_back("Bot");
}
Channel::Channel(std::string name, Client *client): _name(name), _password("x"), _invOnly(false), _topicLock(false), _limit(0)
{
	_clients.push_back((client->getNickName()));
	_admins.push_back(client->getNickName());
}

Channel::Channel(std::string name, std::string password, Client *client): _name(name), _password(password), _invOnly(false), _topicLock(false), _limit(0)
{
	_clients.push_back((client->getNickName()));
	_admins.push_back(client->getNickName());
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

bool Channel::isInvited(std::string nick)
{
	
	for (std::vector<std::string>::iterator i = _invited.begin(); i != _invited.end(); ++i)
	{
		if (*i == nick)
			return true;
	}
	return false;
}

bool Channel::isClient(Client *client)
{
	for (std::vector<std::string>::iterator i = _clients.begin(); i != _clients.end(); ++i)
		if ((*i) == client->getNickName())
			return true;
	return false;
}

bool Channel::isInvOnly()
{
	return _invOnly;
}

void Channel::invite(std::string const &nick)
{
	std::string client(nick);
	_invited.push_back(client);
	
}

bool Channel::isAdmin(std::string nick)
{
	for (std::vector<std::string>::iterator i = _admins.begin(); i != _admins.end(); ++i)
		if (*i == nick)
			return true;
	return false;
}

void Channel::addClient(const std::string &nick)
{
	_clients.push_back(nick);
}

bool Channel::removeClient(std::string nick)
{
	for (std::vector<std::string>::iterator i = _clients.begin(); i != _clients.end(); ++i)
	{
		if ((*i) == nick)
		{
			if (isAdmin(nick))
			 	removeAdmin(nick);
			_clients.erase(i);
			return true;
		}
		
	}
	return false;
}

void Channel::addAdmin(const std::string &nick)
{
	_admins.push_back(nick);
}

void Channel::removeAdmin(std::string nick)
{
	for (std::vector<std::string>::iterator i = _admins.begin(); i != _admins.end(); ++i)
		if ((*i) == nick)
		{
			_admins.erase(i);
			break;
		}
}

void Channel::removeInvited(std::string const &nick)
{
	for (std::vector<std::string>::iterator i = _invited.begin(); i != _invited.end(); ++i)
		if ((*i) == nick)
		{
			_invited.erase(i);
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

std::string Channel::getMode()
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
