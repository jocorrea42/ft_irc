/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LoginCommand.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jocorrea <jocorrea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/11 21:25:22 by fili              #+#    #+#             */
/*   Updated: 2024/09/22 16:51:10 by jocorrea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::_passAutentication(Client *client, const std::vector<std::string> &params)
{
	if (client->getStatus() != PASS)
		_disconnectClient(client, std::string("462 * :" + params[0] + " may not reregister (need PASS state)\r\n"), 0);
	else if (params.size() < 1)
		client->addOutBuffer(std::string("461 " + client->getNickName() + " PASS :Not enough parameters\r\n"));
	else if (params[0] != this->_pass)
		client->addOutBuffer(std::string("464 * :Password " + params[0] + " incorrect " + _pass + "\r\n"));
	else
		client->nextStatus();
}

bool Server::_nickNameOk(const std::string &nickname)
{
	return ((nickname.empty() || nickname.length() > 9 || !std::isalpha(nickname[0])));
}

void Server::_nickAutentication(Client *client, const std::vector<std::string> &params)
{
	std::string oldNick = client->getNickName();
	if (client->getStatus() == REG)
	{
		if (client->getNickName() == (params[0]))
			client->addOutBuffer(std::string(params[0] + " :You are alredy using that nickname.\r\n"));
		else
		{
			changeNickname(client, params[0]);
			_broadcastAllServer(std::string(":" + oldNick + " NICK " + params[0] + "\r\n"));
		}
	}
	else if (client->getStatus() != NICK)
		_disconnectClient(client, std::string("451 * :" + params[0] + " may not reregister (need NICK state)\r\n"), 0);
	else if (params.size() == 0)
		client->addOutBuffer(std::string("431 * :No nickname given\r\n"));
	else if (_nickNameOk(params[0]))
		client->addOutBuffer(std::string("432 * " + params[0] + " :Erroneous nickname\r\n"));
	else if (getClientNick(params[0]))
		client->addOutBuffer(std::string("433 * " + params[0] + " :Nickname is already in use\r\n"));
	else
	{
		client->addOutBuffer(std::string(": " + client->getNickName() + " NICK " + params[0] + "\r\n"));
		client->setNickName(params[0]);
		if (client->getStatus() == NICK)
			client->nextStatus();
	}
}

void Server::_userAutentication(Client *client, const std::vector<std::string> &params)
{

	if (client->getStatus() != USER)
		_disconnectClient(client, std::string("462 * :" + params[0] + " may not reregister (need USER state)\r\n"), 0);
	else if (params.size() < 4)
		client->addOutBuffer(std::string("461 " + client->getNickName() + " USER :Not enough parameters\r\n"));
	else
	{
		std::string nickname = params[0];
		std::string username = params[1];
		std::string servername = params[2];
		std::string realname = params[3];
		client->setUser(username);
		if (client->getNickName() == "Client")
			client->setNickName(nickname);
		client->setName(realname);
		client->addOutBuffer(std::string("001 " + client->getNickName() + " :Welcome to the Internet Relay Network " + client->getNickName() + "!" + username + "@" + realname + "\r\n"));
		client->addOutBuffer(std::string("002 " + client->getNickName() + " :Your host is " + servername + ", running version 1.0\r\n"));
		client->addOutBuffer(std::string("003 " + client->getNickName() + " :This server was created by apodader and jocorrea \"THE PACHANGA TEAM\"" + " \r\n"));
		client->addOutBuffer(std::string("004 " + client->getNickName() + " " + servername + " 1.0 \r\n"));
		client->nextStatus();
		std::vector<std::string> pm;
		pm.push_back(client->getNickName());
		_cmdJoin(client, pm);

	}
}

void Server::_cmdPingSend(Client *client, const std::vector<std::string> &params)
{
	if (params.size() < 1)
		client->addOutBuffer(std::string("409 * :No origin specified \r\n"));
	else
		client->addOutBuffer(std::string("-PONG: no se pmuestra el pong " + params[0] + " \r\n"));
}

void Server::_cmdCap(Client *client, const std::vector<std::string> &params)
{
	if (params.size() && (params[0] == "LS" || params[0] == "END"))
	{
		if (params[0] == "LS")
			client->addOutBuffer(std::string("CAP * LS : \r\n"));
	}
	else
		client->addOutBuffer(std::string("421 * CAP :Unknown command\r\n"));
}

void Server::_cmdQuit(Client *client, const std::vector<std::string> &params)
{
	std::string message = "Client <" + client->getNickName() + "> Quit \r\n";
	if (params.size() >= 1)
		message = params[0];
	_disconnectClient(client, message, 1);
}

void Server::_cmdWho(Client *client, std::vector<std::string> params)
{
	if (params.empty())
	{
		client->addOutBuffer("461 " + client->getNickName() + " WHO :Not enough parameters\r\n");
		return;
	}
	if (!getChannel(params[0]))
	{
		client->addOutBuffer("403 * " + params[0] + " :No such channel\r\n");
		return;
	}
	Channel *channel = getChannel(params[0]);
	if (!channel->isClient(client))
	{
		client->addOutBuffer("442 " + client->getNickName() + " " + params[0] + " :You're not on that channel\r\n");
		return;
	}
	const std::vector<std::string> clients_in_channel = channel->getClients();
	for (std::vector<std::string>::const_iterator it = clients_in_channel.begin(); it != clients_in_channel.end(); ++it)
	{
		Client *user_in_channel = getClientNick(*it);
		std::string user_info = "352 " + client->getNickName() + " " + params[0] + " " + user_in_channel->getName() + " ";
		user_info += "* ";
		user_info += "Server " + user_in_channel->getNickName() + " ";
		if (channel->isAdmin(user_in_channel->getNickName()))
			user_info += "@";

		user_info += " :0 " + user_in_channel->getUser() + "\r\n";
		client->addOutBuffer(user_info);
	}
	client->addOutBuffer("315 " + client->getNickName() + " " + params[0] + " :End of /WHO list.\r\n");
}
