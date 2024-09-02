/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apodader <apodader@student.42barcel>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/28 13:28:31 by jocorrea          #+#    #+#             */
/*   Updated: 2024/09/02 10:55:02 by apodader         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::_cmdMode(Client *client, const std::vector<std::string> &params)
{
	if (client->getStatus() != REG)
		client->addOutBuffer(std::string("451 * :You have not registered \r\n"));
	else if (params.size() < 1)
		client->addOutBuffer(std::string("461 " + client->getNickName() + " MODE :Not enough parameters\r\n"));
	else if (params[0][0] == '#' || params[0][0] == '&')
		_cmdChannelMode(client, params);
	else
		client->addOutBuffer(std::string("502 " + client->getNickName() + " :Cannot change mode for other users\r\n"));
}

void Server::_cmdChannelMode(Client *client, std::vector<std::string> params)
{
	if (Channel *channel = getChannel(params[0]))
	{
		if (params.size() == 1)
			client->addOutBuffer(std::string("324 " + client->getNickName() + " " + params[0] + " " + channel->getMode() + "\r\n"));
		else if (channel->isAdmin(client->getNickName()))
		{
			size_t index = 1;
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
								channel->setPassword(setMode ? params[index++] : "");
							else
							{
								client->addOutBuffer(std::string("461 * " + client->getNickName() + " MODE :Not enough parameters\r\n"));
								return;
							}
							_broadcastClientChannel(channel, std::string(":" + client->getNickName() + " MODE " + params[0] + " " + (setMode ? "+k" : "-k") + " " + (setMode ? channel->getPassword() : "") + "\r\n"), -1);
							index++;
							break;
						case 'o':
							if (index < params.size())
							{
								std::string nickname = params[index++];
								Client *target = getClientNick(nickname);
								if (target)
								{
									(setMode) ? channel->addAdmin(target) : channel->removeAdmin(target->getNickName());
									_broadcastClientChannel(channel, std::string(":" + client->getNickName() + " MODE " + params[0] + " " + (setMode ? "+o" : "-o") + " " + nickname + "\r\n"), -1);
								}
								else
									client->addOutBuffer(std::string("401 " + client->getNickName() + " " + nickname + " :No such nick\r\n"));
							}
							else
								client->addOutBuffer(std::string("461 " + client->getNickName() + " MODE :Not enough parameters\r\n"));
							break;
						case 'l':
							if (setMode && index < params.size())
							{
								channel->setLimit(atoi(params[index++].c_str()));
								_broadcastClientChannel(channel, std::string(":" + client->getNickName() + " MODE " + params[0] + " +l " + params[index - 1] + "\r\n"), -1);
							}
							else if (!setMode)
							{
								channel->setLimit(0);
								_broadcastClientChannel(channel, std::string(":" + client->getNickName() + " MODE " + params[0] + " -l\r\n"), -1);
							}
							else
								client->addOutBuffer("461 " + client->getNickName() + " MODE :Not enough parameters\r\n");
							break;
						default:
							client->addOutBuffer(std::string("472 " + client->getNickName() + " " + std::string(1, modeChar) + " :is unknown mode char to me\r\n"));
							break;
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
	if (params.size() < 1)
		client->addOutBuffer(std::string(ERR_PARAM461));
	else if (Channel *channel = getChannel(params[0]))
	{
		if (!channel->isClient(client))
			client->addOutBuffer(std::string(ERR_CHANN422));
		else if (params.size() == 1)
			(channel->getTopic() == std::string("")) ? client->addOutBuffer(std::string(ERR_TOPIC331)) : client->addOutBuffer(std::string(MSG_TOPIC332));
		else if (!channel->isTopicLocked() || channel->isAdmin(client->getNickName()))
		{
			channel->setTopic(params[1]);
			_broadcastClientChannel(channel, std::string(":" + client->getNickName() + " TOPIC " + params[0] + " :" + channel->getTopic() + "\r\n"), -1);
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
		client->addOutBuffer(std::string(ERR_PARAM461));
	else if (Channel *tChannel = getChannel(params[1]))
	{
		if (tChannel->isClient(client))
		{
			if (tChannel->isAdmin(client->getNickName()))
			{
				Client *target = getClientNick(params[0]);
				if (!target)
					client->addOutBuffer(std::string(ERR_NICKN401));
				else if (tChannel->isClient(target))
					client->addOutBuffer(std::string(ERR_INCHA443));
				else
				{
					tChannel->invite(target->getNickName());
					client->addOutBuffer(std::string("341 " + params[1] + " " + params[0] + "\r\n"));
				}
			}
			else
				client->addOutBuffer(std::string(ERR_OPNEEDED));
		}
		else
			client->addOutBuffer(std::string(ERR_CHANN422));
	}
	else
		client->addOutBuffer(std::string(ERR_NOCHANEL));
}

void Server::_cmdKick(Client *client, std::vector<std::string> params)
{
	if (params.size() < 2)
		client->addOutBuffer(std::string(ERR_PARAM461));
	else if (Channel *channel = getChannel(params[0]))
	{
		if (channel->isAdmin(client->getNickName()))
		{
			std::string reason;
			(params.size() >= 3)? reason = "No reason specified":reason = params[2];
			if (channel->removeClient(client->getNickName()))
					_broadcastClientChannel(channel, std::string(":" + client->getNickName() + " KICK " + params[0] + " " + params[1] + " :" + reason + "\r\n"), -1);
				else
					client->addOutBuffer(std::string("441 " + client->getNickName() + " " + params[1] + " " + params[0] + " :They aren't on that channel\r\n"));
			// for (std::vector<std::string>::iterator i = params.begin() + 1; i != params.end(); ++i)
			// {
			// 	if (channel->removeClient(*i))
			// 		client->addOutBuffer(std::string("User " + *i + " removed from channel " + channel->getName() + "\r\n"));
			// 	else
			// 		client->addOutBuffer(std::string("441 " + client->getNickName() + " " + *i + " " + channel->getName() + " :They aren't on that channel\r\n"));
			// }
		}
		else
			client->addOutBuffer(std::string(ERR_OPNEEDED));
	}
	else
		client->addOutBuffer(std::string(ERR_NOCHANEL));
}

void Server::_cmdJoin(Client *client, const std::vector<std::string> &params)
{
	if (params.empty())
	{
		client->addOutBuffer(std::string("461 " + client->getNickName() + " JOIN :Not enough parameters\r\n"));
		return;
	}
	std::vector<std::string> channels = _splitStr(params[0], ',');
	std::vector<std::string> keys;
	if (params.size() > 1)
		keys = _splitStr(params[1], ',');
	for (size_t i = 0; i < channels.size(); ++i)
	{
		const std::string &channelName = channels[i];
		std::string key = i < keys.size() ? keys[i] : "";
		if (!getChannel(channelName))
		{
			std::vector<std::string> param;
			param.push_back(channelName);
			param.push_back(key);
			addChannel(client, param);
			client->addOutBuffer(std::string(channelName + " created\nAdmin rights granted\r\n"));
			return;
		}
		Channel *channel = getChannel(channelName);
		if (channel->isFull())
			client->addOutBuffer(std::string("471 * " + channel->getName() + " :Cannot join channel (+l)\r\n"));
		else if (channel->isClient(client))
			client->addOutBuffer(std::string("443 " + channel->getName() + " " + client->getNickName() + " :is already on channel\r\n"));
		else if (channel->isInvOnly() && !channel->isInvited(client->getNickName()))
			client->addOutBuffer(std::string("473 * " + channel->getName() + " :Cannot join channel (+i)\r\n"));
		else if ((params.size() > 1 && channel->getPassword() != params[1]) || (params.size() <= 1 && !channel->getPassword().empty()))
			client->addOutBuffer(std::string("475 * " + channel->getName() + " :Cannot join channel (+k)\r\n"));
		else
		{
			channel->addClient(client);
			channel->removeInvited(client->getNickName());
			_broadcastClientChannel(channel, std::string(":" + client->getNickName() + "!~" + client->getName() + " JOIN " + channel->getName() + "\r\n"), client->getFd());
			if (channel->getTopic() == "")
				client->addOutBuffer("331 * " + client->getNickName() + " " + channel->getName() + " :No topic is set" + "\r\n");
			else
				client->addOutBuffer(std::string("332 " + client->getNickName() + " " + channel->getName() + " :" + channel->getTopic() + "\r\n"));
		}
	}
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
					_broadcastClientChannel(channel, std::string(":" + client->getNickName() + " PRIVMSG " + name + " :" + params[1] + " \r\n"), client->getFd());
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
