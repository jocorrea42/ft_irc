/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fili <fili@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/28 13:28:31 by jocorrea          #+#    #+#             */
/*   Updated: 2024/08/28 11:44:45 by fili             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::_cmdMode(Client *client, const std::vector<std::string> &params)
{
	if (client->getStatus() != REG)
		client->addOutBuffer(std::string("451 * :You have not registered \r\n"));
	else if (params.size() < 1)
		client->addOutBuffer(std::string("461 " + client->getNickName() + " MODE :Not enough parameters\r\n"));
	else
	{
		std::string target = params[0];
		if (target[0] == '#' || target[0] == '&') // channel mode
			_cmdChannelMode(client, params);
		else // user mode
			client->addOutBuffer(std::string("502 " + client->getNickName() + " :Cannot change mode for other users\r\n"));
	}
}

void Server::_cmdChannelMode(Client *client, std::vector<std::string> params)
{
	if (Channel *channel = getChannel(params[0]))
	{
		if (params.size() == 1)
			client->addOutBuffer(std::string("324 " + client->getNickName() + " " + params[0] + " " + channel->getMode() + "\r\n"));
		else if (params.size() == 2 && params[1] == "b")
			client->addOutBuffer(std::string("368 " + client->getNickName() + " " + params[0] + " :End of Channel Ban List\r\n"));
		else if (channel->isAdmin(client->getFd()))
		{
			size_t index = 1; // index params
			bool setMode = true;
			while (index < params.size())
			{
				std::string mode = params[index++];
				for (size_t i = 0; i < mode.length(); ++i)
				{
					char modeChar = mode[i];
					if (modeChar == '+' || modeChar == '-')
						setMode = (modeChar == '+');
					else
					{
						switch (modeChar)
						{
						case 'i':
							channel->setInvOnly(setMode);
							client->addOutBuffer(std::string(":" + client->getNickName() + " MODE " + params[0] + (setMode ? "+i" : "-i") + "\r\n"));
							break;
						case 't':
							channel->setTopicLock(setMode);
							client->addOutBuffer(std::string(":" + client->getNickName() + " MODE " + params[0] + " " + (setMode ? "+t" : "-t") + "\r\n"));
							break;
						case 'k':
							if (index < params.size())
							{
								channel->setPassword(setMode ? params[index++] : "");//si el mode es + agrega el siguiente parametro como pass
							}
							else
							{
								client->addOutBuffer(std::string("461 " + client->getNickName() + " MODE :Not enough parameters\r\n"));
								return ;
							}
							_broadcastClientChannel(channel, std::string(":" + client->getNickName() + " MODE " + params[0] + " " + (setMode ? "+k" : "-k") + " " + (setMode ? channel->getPassword() : "") + "\r\n"), -1);
							index++;
							break;
						case 'o':
							for (std::vector<std::string>::iterator i = params.begin() + 2; i != params.end(); ++i)
								channel->giveTakeAdmin(getClientFd(*i), *i, client);
							break;
						case 'l':
							if (params.size() > 2)
							{
								channel->setLimit(atoi(params[2].c_str()));
								client->addOutBuffer(std::string("Limit set to " + params[2] + "\r\n"));
							}
							else
							{
								channel->setLimit(0);
								client->addOutBuffer(std::string("Limit removed\r\n"));
							}
							break;
						default:
							client->addOutBuffer(std::string("472 " + client->getNickName() + " " + params[0] + " : Unknown option\r\n"));
						}
					}
				}
			}
		}
		else
			client->addOutBuffer(std::string(ERR_OPNEEDED));
	}
	else
		client->addOutBuffer(std::string(ERR_NOCHANEL));
}

void Server::_cmdTopic(Client *client, const std::vector<std::string> &params)
{
	if (params.empty() || params.size() > 2)
	{
		client->addOutBuffer(std::string("Usage: TOPIC [channel] [\"new topic\"]\r\n"));
		return;
	}
	if (Channel *channel = getChannel(params[0]))
	{
		if (params.size() == 1)
		{
			client->addOutBuffer(channel->getTopic() + "\r\n");
			return;
		}
		if (!channel->isTopicLocked() || channel->isAdmin(client->getFd()))
		{
			channel->setTopic(params[1]);
			client->addOutBuffer(std::string("Channel topic updated\r\n"));
		}
		else
			client->addOutBuffer(std::string(ERR_OPNEEDED));
	}
	else
		client->addOutBuffer(std::string(ERR_NOCHANEL));
}

void Server::_cmdInvite(Client *client, std::vector<std::string> params)
{
	if (params.size() < 2)
	{
		client->addOutBuffer(std::string("Usage: INVITE [channel] [nickname] [...]\r\n"));
		return;
	}
	if (Channel *channel = getChannel(params[0]))
	{
		if (channel->isAdmin(client->getFd()))
		{
			for (std::vector<std::string>::iterator i = params.begin() + 1; i != params.end(); ++i)
			{
				if (channel->invite(getClientFd(*i)))
					client->addOutBuffer(std::string("User " + *i + " invited\r\n"));
				else
					client->addOutBuffer(std::string("401 " + client->getNickName() + " " + *i + " :No such nick\r\n"));
			}
		}
		else
			client->addOutBuffer(std::string(ERR_OPNEEDED));
	}
	else
		client->addOutBuffer(std::string(ERR_NOCHANEL));
}

void Server::_cmdKick(Client *client, std::vector<std::string> params)
{
	if (params.size() < 2)
	{
		client->addOutBuffer(std::string("Usage: KICK [channel] [nickname] [...]\r\n"));
		return;
	}
	if (Channel *channel = getChannel(params[0]))
	{
		if (channel->isAdmin(client->getFd()))
		{
			for (std::vector<std::string>::iterator i = params.begin() + 1; i != params.end(); ++i)
			{
				if (channel->remove_client(getClientFd(*i)))
					client->addOutBuffer(std::string("User " + *i + " removed from channel " + channel->getName() + "\r\n"));
				else
					client->addOutBuffer(std::string("441 " + client->getNickName() + " " + *i + " " + channel->getName() + " :They aren't on that channel\r\n"));
			}
		}
		else
			client->addOutBuffer(std::string(ERR_OPNEEDED));
	}
	else
		client->addOutBuffer(std::string(ERR_NOCHANEL));
}

void Server::_cmdJoin(Client *client, const std::vector<std::string> &params)
{
	if (client == NULL)
		std::cout << "Client es null\n";
	if (params.empty())
	{
		client->addOutBuffer(std::string("Usage: JOIN [channel] [password]\r\n"));
		return;
	}
	if (!getChannel(params[0]))
	{
		addChannel(client, params);
		client->addOutBuffer(std::string("#" + params[0] + " created\nAdmin rights granted\r\n"));
		return;
	}
	Channel *channel = getChannel(params[0]);
	if (channel->isFull())
	{
		client->addOutBuffer(std::string("471 " + channel->getName() + " :Cannot join channel (+l)\r\n"));
		return;
	}
	if (channel->isClient(client))
		client->addOutBuffer(std::string("443 " + client->getNickName() + " " + channel->getName() + " :is already on channel\r\n"));
	else if (!channel->isInvOnly())
	{
		if (channel->getPassword().empty())
			channel->add_client(client);
		else if (params.size() > 1)
		{
			if (params[1] == channel->getPassword())
				channel->add_client(client);
			else
				client->addOutBuffer(std::string("475 " + channel->getName() + " :Cannot join channel (+k)\r\n"));
		}
	}
	else if (channel->isInvited(client->getFd()))
		channel->add_client(client);
	else
		client->addOutBuffer(std::string("473 " + channel->getName() + " :Cannot join channel (+i)\r\n"));
}

void Server::addChannel(Client *client, const std::vector<std::string> &params)
{
	if (params.size() == 1)
		_channels.push_back(Channel(params[0], client));
	else
		_channels.push_back(Channel(params[0], params[1], client));
}

void Server::_broadcastAllServer(const std::string &message)
{
	for (size_t i = 0; i < _clients.size(); i++)
		_clients[i].addOutBuffer(message);
}

void Server::_cmdPrivmsg(Client *client, std::vector<std::string> params)
{
	int clifd = client->getFd();
	if (client->getStatus() != REG)
	{
		client->addOutBuffer(std::string("451 * :You have not registered \r\n"));
		return;
	}
	if (params.size() < 2)
	{
		client->addOutBuffer(std::string("461 " + client->getNickName() + " PRIVMSG :Not enough parameters \r\n"));
		return;
	}
	std::vector<std::string> obj;
	std::istringstream iss(params[0]);
	std::string token;
	while (std::getline(iss, token, ','))
		obj.push_back(token);
	for (size_t i = 0; i < obj.size(); i++)
	{
		std::string name = obj[i];
		Client *cli = getClientNick(name);
		if (name[0] == '#')
		{
			if (Channel *channel = getChannel(name))
			{
				if (channel->isClient(client))
					_broadcastClientChannel(channel, std::string(":" + client->getNickName() + " PRIVMSG " + name + " :" + params[1] + " \r\n"), clifd);
				else
					client->addOutBuffer(std::string("404 " + client->getNickName() + " " + name + " :Cannot send to channel \r\n"));
			}
			else
				client->addOutBuffer(std::string("403 " + client->getNickName() + " " + name + " :No such channel\r\n"));
		}
		else
		{
			if (cli == NULL)

				client->addOutBuffer(std::string("401 " + client->getNickName() + " " + name + " :No such nick \r\n"));
			else
				cli->addOutBuffer(std::string(":" + client->getNickName() + " PRIVMSG " + name + " :" + params[1] + "\r\n"));
		}
	}
}

void Server::_cmdMsg(Client *client, const std::vector<std::string> &params)
{
	if (params.size() < 2)
	{
		client->addOutBuffer(std::string("Usage: MSG [#channel/nickname] [message]\r\n"));
		return;
	}
	if (params[0][0] == '#')
	{
		if (Channel *channel = getChannel(&params[0][1]))
		{
			if (channel->isClient(client))
				_broadcastAllServer(params[1]);
			else
				client->addOutBuffer(std::string("442 " + channel->getName() + " :You're not on that channel\r\n"));
		}
		else
			client->addOutBuffer(std::string("403 " + client->getNickName() + " " + params[0] + " :No such channel\r\n"));
	}
	else
	{
		if (Client *target = getClientNick(params[0]))
			target->addOutBuffer(std::string(params[1] + "\r\n"));
		else
			client->addOutBuffer(std::string("401 " + client->getNickName() + " " + params[0] + " :No such nick\r\n"));
	}
}