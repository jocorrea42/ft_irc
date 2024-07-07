/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apodader <apodader@student.42barcel>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/21 08:18:50 by fili              #+#    #+#             */
/*   Updated: 2024/06/26 02:07:01 by apodader         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server()
{
	this->_fd = -1;
	this->_polls_size = 0;
	_add.sin_family = AF_INET;		   //-> set the address family to ipv4
	_add.sin_addr.s_addr = INADDR_ANY; //-> set the address to any local machine address
}

Server::Server(int port, std::string password) : _port(port), _pass(password)
{
	this->_fd = -1;
	this->_polls_size = 0;
	_add.sin_family = AF_INET;			//-> set the address family to ipv4
	_add.sin_addr.s_addr = INADDR_ANY;	//-> set the address to any local machine address
	_add.sin_port = htons(this->_port); //-> convert the port to network byte order (big endian)
}

Server::~Server()
{
	for (unsigned short i = 0; i < _polls_size; i++)
		close(_fds[i].fd);
	close(_fd);
	std::cout << "server destroy!!" << std::endl;
}

void Server::ServerStart()
{
	unsigned short revents;
	int opt_val = 1;
	Client *cli;
	// config and create socket
	if ((_fd = socket(_add.sin_family, SOCK_STREAM, 0)) == -1) //-> create the server socket and check if the socket is created
		throw(std::runtime_error("faild to create socket"));
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val)) == -1) //-> set the socket option (SO_REUSEADDR) to reuse the address
		throw(std::runtime_error("faild to set option (SO_REUSEADDR) on socket"));
	if (fcntl(_fd, F_SETFL, O_NONBLOCK) == -1) //-> set the socket option (O_NONBLOCK) for non-blocking socket
		throw(std::runtime_error("faild to set option (O_NONBLOCK) on socket"));
	if (bind(_fd, reinterpret_cast<struct sockaddr *>(&_add), sizeof(_add)) == -1) //-> bind the socket to the address
		throw(std::runtime_error("faild to bind socket"));
	if (listen(_fd, SOMAXCONN) == -1) //-> listen for incoming connections and making the socket a passive socket
		throw(std::runtime_error("listen() faild"));
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
		}
		for (int i = 0; i < _polls_size; i++)
		{
			if (_fds[i].fd != _fd)
			{
				cli = getClient((int)_fds[i].fd);
				if (cli->getOutBuffer().size())
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
	_clients.push_back(*(new Client(inConectionFd, clientadd))); //-> add the client to the vector of clients
	addPollfd(inConectionFd);									 // -> agrega un nuevo fd a la lista de poll para la escucha de un evento
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
		_ClearClient(_fd); //-> clear the client
		return;
	}
	// std::cout << "Buffer: --" << cli->getInBuffer() << " --FIN--" << " r y n : " << cli->getInBuffer().find_first_of("\r\n") << std::endl;
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
		else if (command == std::string("MODE"))
			cli->addOutBuffer(std::string("421") + std::string(" * ") + command + std::string(" :Unknown command\r\n"));
		//_cmdQuit(cli, params);
		else
			cli->addOutBuffer(std::string("421") + std::string(" * ") + command + std::string(" :Unknown command\r\n"));
	}
	cli->sendOwnMessage();
}

void Server::printParam(std::vector<std::string> params)
{
	std::cout << "PARAMS" << std::endl;
	for (size_t i = 0; i < params.size(); ++i)
		std::cout << "|" << params[i] << "|" << std::endl;
	std::cout << "END" << std::endl;
}

void Server::_ClearClient(int fd)
{											 //-> clear the clients
	for (size_t i = 0; i < _fds.size(); i++) //-> remove the client from the pollfd
		if (_fds[i].fd == fd)
		{
			_fds.erase(_fds.begin() + i);
			_polls_size--;
			close(fd);
			break;
		}
	for (size_t i = 0; i < _clients.size(); i++) //-> remove the client from the vector of clients
		if (_clients[i].getFd() == fd)
		{
			_clients.erase(_clients.begin() + i);
			break;
		}
}

void Server::addPollfd(int fd)
{
	struct pollfd newFdClientPoll; // lo mismo, estas tres estructuras se utilizan para la creacion y control delos nuevos clientes conectados
	memset(&newFdClientPoll, 0, sizeof(newFdClientPoll));
	newFdClientPoll.fd = fd;		 //-> add the server socket to the pollfd
	newFdClientPoll.events = POLLIN; //-> set the event to POLLIN for reading data
	_fds.push_back(newFdClientPoll); //-> add the server socket to the pollfd
	_polls_size++;
}

void Server::setFd(int fd)
{
	this->_fd = fd;
}
void Server::setPassword(std::string pass)
{
	this->_pass = pass;
}

void Server::setPort(int port)
{
	this->_port = port;
}

int Server::getPort()
{
	return (this->_port);
}

int Server::getFd()
{
	return (this->_fd);
}

Client *Server::getClient(int fd)
{
	for (size_t i = 0; i < _clients.size(); i++) //-> encuentra el cliente con ese fd
		if (_clients[i].getFd() == fd)
			return (&(this->_clients[i]));
	return (NULL);
}

Client *Server::getClientNick(std::string nickname)
{
	for (size_t i = 0; i < this->_clients.size(); i++)
		if (this->_clients[i].getNickName() == nickname)
			return &this->_clients[i];
	return NULL;
}

void Server::RemoveChannel(std::string name)
{
	for (size_t i = 0; i < this->_channels.size(); i++)
		if (this->_channels[i].GetName() == name)
		{
			this->_channels.erase(this->_channels.begin() + i);
			break;
		}
}

Server &Server::operator=(Server const &other)
{
	this->_add = other._add;
	this->_channels = other._channels;
	this->_clients = other._clients;
	this->_fd = other._fd;
	this->_fds = other._fds;
	this->_pass = other._pass;
	this->_polls_size = other._polls_size;
	this->_port = other._port;
	this->_Signal = other._Signal;
	return (*this);
}

void Server::_CloseFds()
{
	for (size_t i = 0; i < _clients.size(); i++)
	{ //-> close all the clients
		std::cout << "Client <" << _clients[i].getFd() << "> Disconnected" << std::endl;
		close(_clients[i].getFd());
		_clients.erase(_clients.begin() + i);
	}
	if (_fd != -1)
	{ //-> close the server socket
		std::cout << "Server <" << _fd << "> Disconnected" << std::endl;
		close(_fd);
	}
	_clients.clear();
}

bool Server::_Signal = false;
void Server::SignalHandler(int signum)
{
	(void)signum;	// std::cout << std::endl << "Signal Received!" << std::endl;
	_Signal = true; //-> set the static boolean to true to stop the server
}

void Server::_passAutentication(Client *client, std::vector<std::string> params)
{
	if (client->getStatus() != PASS)
	{
		client->addOutBuffer(std::string("462 * :You may not reregister\r\n"));
		return;
	}
	if (params.size() < 1)
	{
		client->addOutBuffer(std::string("461 " + client->getNickName() + " PASS :Not enough parameters\r\n"));
		return;
	}
	if (params[0] != this->_pass)
	{
		client->addOutBuffer(std::string("464 * :Password incorrect\r\n"));
		return;
	}
	std::cout << "pasword ingresado correctamente\n";
	client->nextStatus();
}

bool Server::_nickNameOk(const std::string& nickname)
{
	// vemos que se cumpla el protocolo
	if (nickname.empty() || nickname.length() > 9 || !std::isalpha(nickname[0]))
		return false;
	return true;
}

void Server::_nickAutentication(Client *client, std::vector<std::string> params)
{
	if (client->getStatus() == PASS)
	{
		client->addOutBuffer(std::string("451 * :You have not registered\r\n"));
		return;
	}

	if (params.size() == 0)
	{
		client->addOutBuffer(std::string("431 * :No nickname given\r\n"));
		return;
	}
	if (!_nickNameOk(params[0]))
	{
		client->addOutBuffer(std::string("432 * " + params[0] + " :Erroneous nickname\r\n"));
		return;
	}
	if (client->getStatus() == REG)
		return;
	client->addOutBuffer(std::string(": " + client->getNickName() + " NICK " + params[0] + "\r\n"));
	//_broadcast_to_all_clients_on_server(nick_change_msg);
	client->setNickName(params[0]);
	if (client->getStatus() == NICK)
		client->nextStatus();
}

void Server::_userAutentication(Client *client, std::vector<std::string> params)
{
	if (client->getStatus() != USER)
	{
		client->addOutBuffer(std::string("462 * :You may not reregister\r\n"));
		return;
	}

	if (params.size() < 4)
	{
		client->addOutBuffer("461 " + client->getNickName() + " USER :Not enough parameters\r\n");
		return;
	}
	std::string nickname = params[0];
	std::string username = params[1];
	std::string realname = params[3];
	std::string servername = params[2];
	// Ignore hostname and servername when USER comes from a directly connected client.
	client->setUser(username);
	client->setNickName(nickname);
	client->setName(realname);
	//std::cout << "BIENVENIDO " << client->getUser() << std::endl;
	std::string rpl_welcome_msg = "001 " + nickname + " :Welcome to the Internet Relay Network " + nickname + "!" + username + "@" + realname + "\r\n";
	client->addOutBuffer(std::string(rpl_welcome_msg));
	// Send RPL_YOURHOST: 002
	std::string rpl_yourhost_msg = "002 " + nickname + " :Your host is " + servername + ", running version 1.0\r\n";
	client->addOutBuffer(std::string(rpl_yourhost_msg));
	// Send RPL_CREATED: 003
	std::string rpl_created_msg = "003 " + nickname + " :This server was created POR GERONIMA" + "\r\n";
	client->addOutBuffer(std::string(rpl_created_msg));
	// Send RPL_MYINFO: 004
	std::string rpl_myinfo_msg = "004 " + nickname + " " + servername + " 1.0 o o\r\n";
	client->addOutBuffer(std::string(rpl_myinfo_msg));
	client->nextStatus();
}

void Server::_cmdPingSend(Client *client, std::vector<std::string> params)
{
	std::string answer = params[0];
	std::string response = "PONG " + answer + "\r\n";
	client->addOutBuffer(response);
	// client->sendOwnMessage();
	// client->cleanOutBuffer();
}

void Server::_cmdCap(Client *client, std::vector<std::string> params)
{
	if (params.size() && (params[0] == "LS" || params[0] == "END"))
	{
		if (params[0] == "LS") // si es END no hace nada
			client->addOutBuffer(std::string("CAP * LS :\r\n"));
	}
	else
		client->addOutBuffer(std::string("421 * CAP :Unknown command\r\n"));
}

void Server::_cmdQuit(Client *client, std::vector<std::string> params)
{
	std::string message = "Client Quit";
	if (params.size() >= 1)
		message = params[0];
	client->setOutBuffer(message);
}