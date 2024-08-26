/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LoginCommand.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fili <fili@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/11 21:25:22 by fili              #+#    #+#             */
/*   Updated: 2024/08/11 21:26:50 by fili             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::_passAutentication(Client *client, const std::vector<std::string> &params)
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

void Server::_nickAutentication(Client *client, const std::vector<std::string> &params)
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

void Server::_userAutentication(Client *client, const std::vector<std::string> &params)
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
		if (params[0] == "LS") // si es END no hace nada
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
	client->setOutBuffer(message);
	close(client->getFd());
	_disconnectClient(client, message);
	
}