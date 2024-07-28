/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jocorrea <jocorrea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/21 08:18:50 by fili              #+#    #+#             */
/*   Updated: 2024/07/28 16:40:11 by jocorrea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void Server::ServerStart()
{
	unsigned short revents;
	int opt_val = 1;
	Client *cli;
	// config and create socket
	if ((_fd = socket(_add.sin_family, SOCK_STREAM, 0)) == -1) //-> create the server socket and check if the socket is created
		throw(std::runtime_error("failed to create socket"));
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val)) == -1) //-> set the socket option (SO_REUSEADDR) to reuse the address
		throw(std::runtime_error("failed to set option (SO_REUSEADDR) on socket"));
	if (fcntl(_fd, F_SETFL, O_NONBLOCK) == -1) //-> set the socket option (O_NONBLOCK) for non-blocking socket
		throw(std::runtime_error("failed to set option (O_NONBLOCK) on socket"));
	if (bind(_fd, reinterpret_cast<struct sockaddr *>(&_add), sizeof(_add)) == -1) //-> bind the socket to the address
		throw(std::runtime_error("failed to bind socket"));
	if (listen(_fd, SOMAXCONN) == -1) //-> listen for incoming connections and making the socket a passive socket
		throw(std::runtime_error("listen() failed"));
	addPollfd(_fd);
	std::cout << "IRC SERVER <" << _fd << "> I AM ALIVE" << std::endl;
	std::cout << "SERVER WAIT FOR CLIENT CONNECTIONS .......\n";
	while (!(this->_Signal))											 //-> run the server until the signal is received
	{																	 // poll(fdsarray, fdsarraysize, time) el time en -1 bloquea hasta que exita evento en el poll
		if ((poll(&_fds[0], _polls_size, -1) == -1) && _Signal == false) // codigo para ver si ocurrio un evento
			throw(std::runtime_error("poll() fail, el vento salio mal"));
		for (int i = 0; i < _polls_size; i++) // miro todos los fds en el poll vector
		{
			revents = _fds[i].revents;
			if (revents == 0)
				continue;
			if ((revents & POLLERR) == POLLERR || (revents & POLLHUP) == POLLHUP)
			{
				std::cout << "unexpected client disconnection\n";
				_ClearClient(_fds[i].fd);
				continue;
			}
			if (revents & POLLIN) // si el and es uno es por que revents es POLLIN o sea hay entrada para ser leida
			{
				if (_fds[i].fd == _fd) // en este caso es una peticion al server de un cliente
					AcceptNewClient();
				else // en este caso es un mensaje de algun cliente
					ReceiveNewData(_fds[i].fd);
			}
			for (int i = 0; i < _polls_size; i++)
			{
				if (_fds[i].fd != _fd)
				{
					cli = getClient((int)_fds[i].fd);
					if (cli != NULL && cli->getOutBuffer().size())
					{
						if (cli->sendOwnMessage())
						{
							cli->cleanOutBuffer();
						}
						else
						{
							std::cout << "   -----unexpected client disconnection\n";
							_ClearClient(_fds[i].fd);
						}
					}
				}
			}
		}
	}
	_CloseFds();
}

void Server::AcceptNewClient() // agregamos un  cliente a la lista de clientes
{
	std::cout << "NEW CLIENT CONNECTION !!!!\n";
	int inConectionFd;
	struct sockaddr_in clientadd; // lo mismo pero para un nuevo cliente conectado
	socklen_t len = sizeof(clientadd);

	if ((inConectionFd = accept(_fd, (sockaddr *)&(clientadd), &len)) == -1) //-> accept the new client
		throw(std::runtime_error("faild accept client"));
	if (fcntl(inConectionFd, F_SETFL, O_NONBLOCK) == -1) //-> set the socket option (O_NONBLOCK) for non-blocking socket
		throw(std::runtime_error("faild to set option (O_NONBLOCK) on socket of client"));
	_clients.push_back((Client(inConectionFd, clientadd))); //-> add the client to the vector of clients
	addPollfd(inConectionFd);								// -> agrega un nuevo fd a la lista de poll para la escucha de un evento
	std::cout << "CLIENT <" << inConectionFd << "> IS CONNECTED!!!" << std::endl;
}

void Server::ReceiveNewData(int fd)
{
	std::string command;
	std::string sms;
	std::vector<std::string> params;
	std::string token;
	Client *cli = getClient(fd);

	if (!cli->receiveMessage() || cli->getInBuffer().find("\r\n") == std::string::npos)
	{
		_ClearClient(cli->getFd()); //-> clear the client
		return;
	}
	std::string line = cli->getInBuffer();
	while (line.size() > 0)
	{
		sms = line.substr(0, line.find("\r\n"));
		line.erase(0, line.find("\r\n") + 2);
		std::cout << "Message from " << cli->getNickName() << ": " << sms << std::endl;
		cli->cleanInBuffer();
		params.clear();
		// Extraer parametros y comandos
		std::istringstream iss(sms);
		if (sms[0] == ':')
			iss >> token; // Read and discard the prefix
		// extrae el comando
		iss >> command;
		// extraemos los parametros
		while (iss >> token)
		{
			if (token[0] == ':')
			{ // Extract the trailing part
				std::string trailing;
				std::getline(iss, trailing);				  // extrae todo el texto
				params.push_back(token.substr(1) + trailing); // quita los dos puntos
				break;
			}
			else
				params.push_back(token); // el parametro no tiene :
		} // Remove trailing \r from the last parameter
		if (!params.empty() && !params[params.size() - 1].empty() && params[params.size() - 1][params[params.size() - 1].size() - 1] == '\r')
			params[params.size() - 1].resize(params[params.size() - 1].size() - 1);
		// printParam(params);
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
		else if (command == "MSG")
			_cmdMsg(cli, params);
		else if (command == "KICK")
			_cmdKick(cli, params);
		else if (command == "INVITE")
			_cmdInvite(cli, params);
		else if (command == "TOPIC")
			_cmdTopic(cli, params);
		else if (command == std::string("MODE"))
			_cmdMode(cli, params);
		else
			cli->addOutBuffer(std::string("421") + std::string(" * ") + command + std::string(" :Unknown command\r\n"));
	}
}