/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jocorrea <jocorrea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/21 08:18:50 by fili              #+#    #+#             */
/*   Updated: 2024/09/22 17:01:58 by jocorrea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::ServerStart()
{
	unsigned short revents;
	int opt_val = 1;
	Client *cli;
	if ((_fd = socket(_add.sin_family, SOCK_STREAM, 0)) == -1)
		throw(std::runtime_error("failed to create socket"));
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val)) == -1)
		throw(std::runtime_error("failed to set option (SO_REUSEADDR) on socket"));
	if (fcntl(_fd, F_SETFL, O_NONBLOCK) == -1)
		throw(std::runtime_error("failed to set option (O_NONBLOCK) on socket"));
	if (bind(_fd, reinterpret_cast<struct sockaddr *>(&_add), sizeof(_add)) == -1)
		throw(std::runtime_error("failed to bind socket"));
	if (listen(_fd, 64) == -1)
		throw(std::runtime_error("listen() failed"));
	addPollfd(_fd);
	std::cout << "Server is running" << std::endl;
	while (!(this->_Signal))
	{
		if ((poll(&_fds[0], _polls_size, 200) == -1) && _Signal == false)
			throw(std::runtime_error("poll() fail, el vento salio mal"));
		for (int i = 0; i < _polls_size; i++)
		{
			revents = _fds[i].revents;
			if (revents == 0)
				continue;
			if ((revents & POLLERR) == POLLERR || (revents & POLLHUP) == POLLHUP)
			{
				_ClearClient(_fds[i].fd);
				continue;
			}
			if (revents & POLLIN)
			{
				if (_fds[i].fd == _fd)
					AcceptNewClient();
				else
					ReceiveNewData(_fds[i].fd);
				for (size_t j = 1; j < _polls_size; j++)
					_fds[j].events |= POLLOUT;
			}
			if (_fds[i].revents & POLLOUT)
			{
				if (_fds[i].fd != _fd)
				{
					cli = getClient((int)_fds[i].fd);
					if (cli)
					{
						if (cli->sendOwnMessage())
							cli->cleanOutBuffer();
						else
							_disconnectClient(cli, std::string("unexpected client disconnection send msg to " + cli->getNickName() + "\n"), 1);
					}
				}
				_fds[i].events &= ~POLLOUT;
			}
		}
	}
	_CloseFds();
}

void Server::_addClient(int inConectionFd, struct sockaddr_in clientadd)
{
	_clients.push_back((Client(inConectionFd, clientadd)));
}

void Server::AcceptNewClient()
{
	int inConectionFd;
	struct sockaddr_in clientadd;
	socklen_t len = sizeof(clientadd);

	if ((inConectionFd = accept(_fd, (sockaddr *)&(clientadd), &len)) == -1)
		throw(std::runtime_error("faild accept client"));
	if (fcntl(inConectionFd, F_SETFL, O_NONBLOCK) == -1)
		throw(std::runtime_error("faild to set option (O_NONBLOCK) on socket of client"));
	_addClient(inConectionFd, clientadd);
	addPollfd(inConectionFd);
}

void Server::ReceiveNewData(int fd)
{
	std::string command;
	std::string sms;
	std::vector<std::string> params;
	std::string token;
	Client *cli = getClient(fd);

	if (!cli->receiveMessage())
		_disconnectClient(cli, std::string("Client disconected " + cli->getInBuffer()), 1);
	else if (!cli->msgLon())
	{
		if (cli->getInBuffer().find(std::string("\r\n")) != std::string::npos || cli->getInBuffer().find(std::string("\n")) != std::string::npos)
		{
			std::string match = "\n";
			std::string line = cli->getInBuffer();
			cli->cleanInBuffer();
			while (line.size() > 0)
			{
				if (line.find(std::string("\r\n")) != std::string::npos)
					match = "\r\n";
				sms = line.substr(0, line.find(match));
				line.erase(0, line.find(match) + match.length());
				std::cout << "Msg from: " << cli->getNickName() << ": " << sms << std::endl;
				params.clear();
				std::istringstream iss(sms);
				if (sms[0] == ':')
					iss >> token;
				iss >> command;
				while (iss >> token)
				{
					if (token[0] == ':')
					{
						std::string trailing;
						std::getline(iss, trailing);
						params.push_back(token.substr(1) + trailing);
						break;
					}
					else
						params.push_back(token);
				}
				if (!params.empty() && !params[params.size() - 1].empty() && params[params.size() - 1][params[params.size() - 1].size() - 1] == '\r')
					params[params.size() - 1].resize(params[params.size() - 1].size() - 1);
				if (command == std::string("PASS"))
					_passAutentication(cli, params);
				else if (command == std::string("NICK"))
					_nickAutentication(cli, params);
				else if (command == std::string("USER"))
					_userAutentication(cli, params);
				else if (command == std::string("PING"))
					_cmdPingSend(cli, params);
				else if (command == std::string("CAP"))
					_cmdCap(cli, params);
				else if (command == std::string("QUIT"))
					_cmdQuit(cli, params);
				else if (command == "JOIN")
					_cmdJoin(cli, params);
				else if (command == "PRIVMSG")
					_cmdPrivmsg(cli, params);
				else if (command == "KICK")
					_cmdKick(cli, params);
				else if (command == "INVITE")
					_cmdInvite(cli, params);
				else if (command == "TOPIC")
					_cmdTopic(cli, params);
				else if (command == std::string("MODE"))
					_cmdMode(cli, params);
				else if (command == std::string("WHOIS"))
					_cmdWho(cli, params);
				else
				{
					cli->addOutBuffer(std::string("421") + std::string(" * ") + command + std::string(" :Unknown command\r\n"));
					break;
				}
			}
		}
	}
	else
		cli->addOutBuffer(std::string("417 * :Input line was too long\r\n"));
}