/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jocorrea <jocorrea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/28 13:28:31 by jocorrea          #+#    #+#             */
/*   Updated: 2024/07/28 14:56:50 by jocorrea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::_passAutentication(Client *client, std::vector<std::string> params)
{
	if (client->getStatus() != PASS)
		client->addOutBuffer(std::string("462 * :You may not register\r\n"));
	else if (params.size() < 1)
		client->addOutBuffer(std::string("461 " + client->getNickName() + " PASS :Not enough parameters\r\n"));
	else if (params[0] != this->_pass)
		client->addOutBuffer(std::string("464 * :Password incorrect\r\n"));
	else
	{
		std::cout << client->getFd() << ": a ingresado al server correctamente\n";
		client->nextStatus();
	}
}

bool Server::_nickNameOk(const std::string &nickname)
{
	// vemos que se cumpla el protocolo
	if (nickname.empty() || nickname.length() > 9 || !std::isalpha(nickname[0]))
		return false;
	return true;
}

void Server::_nickAutentication(Client *client, std::vector<std::string> params)
{
	std::string oldNick = client->getNickName();
	if (client->getStatus() == PASS)
		client->addOutBuffer(std::string("451 * :You have not registered\r\n"));
	else if (params.size() == 0)
		client->addOutBuffer(std::string("431 * :No nickname given\r\n"));
	else if (!_nickNameOk(params[0]))
		client->addOutBuffer(std::string("432 * " + params[0] + " :Erroneous nickname\r\n"));
	else if (getClientNick(params[0]))
		client->addOutBuffer(std::string("433 * " + params[0] + " :Nickname is already in use\r\n"));
	else if (client->getStatus() == REG)
	{
		client->setNickName(params[0]);
		_broadcastAllServer(std::string(":" + oldNick + " NICK " + params[0] + "\r\n"));
	}
	else
	{
		client->addOutBuffer(std::string(": " + client->getNickName() + " NICK " + params[0] + "\r\n"));
		client->setNickName(params[0]);
		if (client->getStatus() == NICK)
			client->nextStatus();
	}
}

void Server::_userAutentication(Client *client, std::vector<std::string> params)
{
	if (client->getStatus() != USER)
		client->addOutBuffer(std::string("462 * :You may not reregister\r\n"));
	if (params.size() < 4)
		client->addOutBuffer(std::string("461 " + client->getNickName() + " USER :Not enough parameters\r\n"));
	else
	{
		std::string nickname = params[0];
		std::string username = params[1];
		std::string servername = params[2];
		std::string realname = params[3];
		// Ignore hostname and servername when USER comes from a directly connected client.
		client->setUser(username);
		client->setNickName(nickname);
		client->setName(realname);
		client->addOutBuffer(std::string("001 " + nickname + " :Welcome to the Internet Relay Network " + nickname + "!" + username + "@" + realname + "\r\n"));
		// Send RPL_YOURHOST: 002
		client->addOutBuffer(std::string("002 " + nickname + " :Your host is " + servername + ", running version 1.0\r\n"));
		// Send RPL_CREATED: 003
		client->addOutBuffer(std::string("003 " + nickname + " :This server was created for apodader and jocorrea \"THE PACHANGA TEAM\"" + " \r\n"));
		// Send RPL_MYINFO: 004
		client->addOutBuffer(std::string("004 " + nickname + " " + servername + " 1.0 \r\n"));
		client->nextStatus();
	}
}

void Server::_cmdPingSend(Client *client, std::vector<std::string> params)
{
	if (params.size() < 1)
		client->addOutBuffer(std::string("409 * :No origin specified \r\n"));
	else
		client->addOutBuffer(std::string("-PONG: no se pmuestra el pong " + params[0] + " \r\n"));
}

void Server::_cmdCap(Client *client, std::vector<std::string> params)
{
	if (params.size() && (params[0] == "LS" || params[0] == "END"))
	{
		if (params[0] == "LS") // si es END no hace nada
			client->addOutBuffer(std::string("CAP * LS : \r\n"));
	}
	else
		client->addOutBuffer(std::string("421 * CAP :Unknown command\r\n"));
}

void Server::_cmdQuit(Client *client, std::vector<std::string> params)
{
	std::string message = "Client <" + client->getNickName() + "> Quit \r\n";
	if (params.size() >= 1)
		message = params[0];
	client->setOutBuffer(message);
}

void Server::_cmdMode(Client *client, std::vector<std::string> params)
{
	if (client->getStatus() != REG)
		client->addOutBuffer(std::string("451 * :You have not registered \r\n"));
	else if (params.size() < 1)
		client->addOutBuffer(std::string("461 " + client->getNickName() + " MODE :Not enough parameters\r\n"));
	else
	{
		std::string target = params[0];
		if (target[0] == '#' || target[0] == '&') // Channel mode
			_cmdChannelMode(client, params);
		else // User mode
			client->addOutBuffer(std::string("502 " + client->getNickName() + " :Cannot change mode for other users\r\n"));
	}
}

void Server::_cmdChannelMode(Client *client, std::vector<std::string> params)
{
	if (params.size() < 2 || params[0].size() != 2)
		client->addOutBuffer(std::string("Usage: MODE [-i/-t/-k/-o/-l] [channel] [arguments] \r\n"));
	else if (Channel *channel = getChannel(params[1]))
	{
		if (channel->isAdmin(client->getFd()))
		{
			switch (params[0][1])
			{
			case 'i':
				if (channel->isInvOnly())
				{
					channel->unsetInvOnly();
					client->addOutBuffer(std::string("Invite-only channel disabled for " + params[1] + "\r\n"));
				}
				else
				{
					channel->setInvOnly();
					client->addOutBuffer(std::string("Invite-only channel abled for " + params[1] + "\r\n"));
				}
				break;
			case 't':
				if (channel->isTopicLocked())
				{
					channel->unsetTopicLock();
					client->addOutBuffer(std::string("Topic change rights allowed to non-operators for " + params[1] + "\r\n"));
				}
				else
				{
					channel->setTopicLock();
					client->addOutBuffer(std::string("Topic change rights restricted to operators for " + params[1] + "\r\n"));
				}
				break;
			case 'k':
				if (params.size() > 2)
				{
					channel->SetPassword(params[2]);
					client->addOutBuffer(std::string("Password set\r\n"));
				}
				else
				{
					channel->SetPassword(NULL);
					client->addOutBuffer(std::string("Password removed\r\n"));
				}
				break;
			case 'o':
				for (std::vector<std::string>::iterator i = params.begin() + 2; i != params.end(); ++i)
					channel->GiveTakeAdmin(getClientFd(*i), *i, client);
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
				client->addOutBuffer(std::string("Unknown option: " + params[0] + "\r\n"));
			}
		}
		else
			client->addOutBuffer(std::string("You don't have admin rights\r\n"));
	}
	else
		client->addOutBuffer(std::string("Channel " + params[0] + " does not exist\r\n"));
}

void Server::_cmdTopic(Client *client, std::vector<std::string> params)
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
			client->addOutBuffer(std::string("You don't have admin rights\r\n"));
	}
	else
		client->addOutBuffer(std::string("Channel " + params[0] + " does not exist\r\n"));
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
					client->addOutBuffer(std::string("User " + *i + " does not exist\r\n"));
			}
		}
		else
			client->addOutBuffer(std::string("You don't have admin rights\r\n"));
	}
	else
		client->addOutBuffer(std::string("Channel " + params[0] + " does not exist\r\n"));
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
					client->addOutBuffer(std::string("User " + *i + " removed from channel " + channel->GetName() + "\r\n"));
				else
					client->addOutBuffer(std::string("User " + *i + " does not exist on channel " + channel->GetName() + "\r\n"));
			}
		}
		else
			client->addOutBuffer(std::string("You don't have admin rights\r\n"));
	}
	else
		client->addOutBuffer(std::string("Channel " + params[0] + " does not exist\r\n"));
}

void Server::_cmdMsg(Client *client, std::vector<std::string> params)
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
			if (channel->isClient(client->getFd()))
				channel->sendToAll(params[1], client->getFd());
			else
				client->addOutBuffer(std::string("You do not belong to this channel\r\n"));
		}
		else
			client->addOutBuffer(std::string("Channel " + params[0] + " does not exist\r\n"));
	}
	else
	{
		if (Client *target = getClientNick(params[0]))
			target->addOutBuffer(std::string(params[1] + "\r\n"));
		else
			client->addOutBuffer(std::string("User " + params[0] + " does not exist\r\n"));
	}
}

void Server::_cmdJoin(Client *client, std::vector<std::string> params)
{
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
		client->addOutBuffer(std::string("#" + params[0] + " is full\r\n"));
		return;
	}
	if (channel->isClient((int)client->getFd()))
		client->addOutBuffer(std::string("You alredy joined this channel\r\n"));
	else if (!channel->isInvOnly())
	{
		if (channel->GetPassword().empty())
			channel->add_client(client);
		else if (params.size() > 1)
		{
			if (params[1] == channel->GetPassword())
				channel->add_client(client);
			else
				client->addOutBuffer(std::string("Incorrect password\r\n"));
		}
	}
	else if (channel->isInvited(client->getFd()))
		channel->add_client(client);
	else
		client->addOutBuffer(std::string("You need an invitation in order to join this channel\r\n"));
}

void Server::addChannel(Client *client, const std::vector<std::string> &params)
{
	if (params.size() == 1)
		_channels.push_back(Channel(params[0], client));
	else
		_channels.push_back(Channel(params[0], params[1], client));
}

void Server::_broadcastAllServer(std::string message)
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
				if (channel->isClient(client->getFd()))
					channel->sendToAll(std::string(":" + client->getNickName() + " PRIVMSG " + name + " :" + params[1] + " \r\n"), client->getFd());
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