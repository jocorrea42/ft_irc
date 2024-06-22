/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fili <fili@student.42.fr>                  #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024-05-21 08:18:50 by fili              #+#    #+#             */
/*   Updated: 2024-05-21 08:18:50 by fili             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"

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

	std::cout << "IRC SERVER <" << _fd << "> is listening!!!!!!!!" << std::endl;
	std::cout << "Wait for clients...\n";

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
				break;
			}
			if (revents & POLLIN) // si el and es uno es por que revents es POLLIN o sea hay entrada para ser leida
			{
				if (_fds[i].fd == _fd) // en este caso es una peticion al server de un cliente
					AcceptNewClient();
				else // en este caso es un mensaje de algun cliente
					ReceiveNewData(_fds[i].fd);
			}
		}
	}
	_CloseFds();
}

void Server::AcceptNewClient() // agregamos un  cliente a la lista de clientes
{
	std::cout << "Client conection!!!!!\n";
	int inConectionFd;
	socklen_t len = sizeof(_clientadd);

	if ((inConectionFd = accept(_fd, (sockaddr *)&(_clientadd), &len)) == -1) //-> accept the new client
		throw(std::runtime_error("faild accept client"));
	if (fcntl(inConectionFd, F_SETFL, O_NONBLOCK) == -1) //-> set the socket option (O_NONBLOCK) for non-blocking socket
		throw(std::runtime_error("faild to set option (O_NONBLOCK) on socket of client"));
	_clients.push_back(Client(inConectionFd, inet_ntoa((_clientadd.sin_addr)))); //-> add the client to the vector of clients
	addPollfd(inConectionFd);													 // -> agrega un nuevo fd a la lista de poll para la escucha de un evento
	std::cout << "Client <" << inConectionFd << "> Connected!!!" << std::endl;
}

void Server::ReceiveNewData(int fd)
{
	std::string command;
	std::string sms;
	std::vector<std::string> params;
	std::string token;
	std::string trailing;
	Client *cli = getClient(fd);
	char buff[1024] = {0}; //-> buffer for the received data

	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1, 0); //-> receive the data
	if (bytes <= 0)
	{ //-> check if the client disconnected
		std::cout << "Client <" << fd << "> Disconnected" << std::endl;
		_ClearClient(fd); //-> clear the client
	}
	else
	{ //-> print the received data
		cli->addBuffer(buff);
		if (cli->getBuffer().find_first_of("\r\n") == std::string::npos)
			return;
		sms = cli->getBuffer().substr(0, cli->getBuffer().find("\r\n"));
		// Extraer parametros y comandos
		std::istringstream iss(sms);
		if (sms[0] == ':')
			iss >> token; // Read and discard the prefix
		// extrae el comando
		iss >> command;
		// extraemos los parametros
		while (iss >> token)
		{
			std::cout << "entro con token " << token << std::endl;
			if (token[0] == ':')
			{												  // Extract the trailing part
				std::getline(iss, trailing);				  // extrae todo el texto
				params.push_back(token.substr(1) + trailing); // quita los dos puntos
				break;
			}
			else
				params.push_back(token); // el parametro no tiene :
		} // Remove trailing \r from the last parameter
		if (!params.empty() && !params[params.size() - 1].empty() && params[params.size() - 1][params[params.size() - 1].size() - 1] == '\r')
			params[params.size() - 1].resize(params[params.size() - 1].size() - 1);
		printParam(params);
		if (command == std::string("PASS"))
			_passAutentication(cli, params);
		if (command == std::string("NICK"))
			_nickAutentication(cli, params);
		if (command == std::string("USER"))
			_userAutentication(cli, params);
		if (command == std::string("PING"))
			_cmdPingSend(cli, params);			
		
	}
}
void	Server::printParam(std::vector<std::string> params)
{
	std::cout << "PARAMS" << std::endl;
	for (size_t i = 0; i < params.size(); ++i)
		std::cout << "|" << params[i] << "|" << std::endl;
	std::cout << "END" << std::endl;
}

void Server::_ClearClient(int fd)
{ //-> clear the clients
	for (size_t i = 0; i < _fds.size(); i++)//-> remove the client from the pollfd
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
	this->_clientadd = other._clientadd;
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
	}
	if (_fd != -1)
	{ //-> close the server socket
		std::cout << "Server <" << _fd << "> Disconnected" << std::endl;
		close(_fd);
	}
}

bool Server::_Signal = false;
void Server::SignalHandler(int signum)
{
	(void)signum;
	std::cout << std::endl << "Signal Received!" << std::endl;
	_Signal = true; //-> set the static boolean to true to stop the server
}

void Server::_passAutentication(Client *client, std::vector<std::string> params)
{
	if (client->getStatus() != PASS)
	{
		std::cout << "esta en otro estado:" << client->getStatus() << std::endl;
		return;
	}
	if (params.size() < 1)
	{
		std::cout << "los parmetros estan mal, no hay\n";
		return;
	}
	if (params[0] != this->_pass)
	{
		std::cout << "pasword incorrecto\n";
		return;
	}
	std::cout << "pasword ingresado correctamente\n";
	client->nextStatus();

}

void Server::_nickAutentication(Client *client, std::vector<std::string> params)
{
	if (client->getStatus() == PASS)
	{
		std::cout << "esta en otro estado:" << client->getStatus() << std::endl;
		return;
	}
	if (params.size() == 0)
	{
		std::cout << "los parmetros estan mal, no hay\n";
		return;
	}
	std::string new_nickname = params[0];
	if (client->getStatus() == REG)
	{
		// Notify other users in the same channels about the nickname change
		std::string nick_change_msg = ":" + client->getNickName() + " NICK " + new_nickname + "\r\n";
		//_broadcast_to_all_clients_on_server(nick_change_msg);
		client->setNickName(new_nickname);
		return;
	}

	// if status is nick we proceed to user
	if (client->getStatus() == NICK)
	{
		client->setNickName(new_nickname);
		std::cout << "hola " << new_nickname << std::endl;
		client->nextStatus();
	}
}

void		Server::_userAutentication(Client *client,std::vector<std::string> params)
{
	if (client->getStatus() != USER)
	{
		std::cout << "esta en otro estado:" << client->getStatus() << std::endl;
		return;
	}

	if (params.size() < 4)
	{
		std::cout << "los parmetros estan mal, no hay\n";
		return;
	}

	std::string username = params[0];
	std::string hostname = params[1];
	std::string servername = params[2];
	std::string realname = params[3];

	// Ignore hostname and servername when USER comes from a directly connected client.
	// They will be used only in server-to-server communication.

	// Set the user's username and realname
	client->setUser(username);
	client->setName(realname);
	std::cout << "BIENVENIDO " << client->getUser() << std::endl;
	client->nextStatus();
}

void		Server::_cmdPingSend(Client *client, std::vector<std::string> params)
{
	if (params.size() < 1)
	{
		std::cout << "los parmetros estan mal, no hay\n";
		return;
	}

	std::string answer = params[0];
	std::string response = "PONG " + answer + "\r\n";
	// Send Pong
	client->sendMessage(response);
}