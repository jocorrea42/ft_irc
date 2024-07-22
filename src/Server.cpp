/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apodader <apodader@student.42barcel>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/21 08:18:50 by fili              #+#    #+#             */
/*   Updated: 2024/07/22 21:56:17 by apodader         ###   ########.fr       */
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
			cli->addOutBuffer(std::string("421") + std::string(" * ") + command + std::string(" :Unknown command\r\n"));
		//_cmdQuit(cli, params);
		else
			cli->addOutBuffer(std::string("421") + std::string(" * ") + command + std::string(" :Unknown command\r\n"));
	}
	cli->sendOwnMessage();
}

void Server::_cmdChannelMode(Client *client, std::vector<std::string> params)
{
	if (params.size() < 2 || params[0].size() != 2)
	{
		client->addOutBuffer(std::string("Usage: MODE [-i/-t/-k/-o/-l] [channel] [arguments]\r\n"));
		return;
	}
	if (Channel *channel = getChannel(params[1]))
	{
		if (channel->isAdmin(client->getFd()))
		{
			switch(params[0][1]){
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
				channel->sendToAll(params[1]);
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
	if (channel->isClient(client->getFd()))
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
	for (std::vector<Client>::iterator i = _clients.begin(); i != _clients.end(); ++i)
		if ((*i).getFd() == fd)
			return &(*i);
	return (NULL);
}

Client *Server::getClientNick(std::string nickname)
{
	for (std::vector<Client>::iterator i = _clients.begin(); i != _clients.end(); ++i)
		if ((*i).getNickName() == nickname)
			return &(*i);
	return NULL;
}

int Server::getClientFd(const std::string &nick)
{
	for (std::vector<Client>::iterator i = _clients.begin(); i != _clients.end(); ++i)
		if ((*i).getNickName() == nick)
			return (*i).getFd();
	return -1;
}

Channel *Server::getChannel(const std::string &name)
{
	for (std::vector<Channel>::iterator i = _channels.begin(); i != _channels.end(); ++i)
		if (i->GetName() == name)
			return &(*i);
	return NULL;
}

void Server::RemoveChannel(const std::string &name)
{
	for (std::vector<Channel>::iterator i = _channels.begin(); i != _channels.end(); ++i)
		if (i->GetName() == name)
		{
			_channels.erase(i);
			return;
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
		client->addOutBuffer(std::string("462 * :You may not register\r\n"));
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

bool Server::_nickNameOk(const std::string &nickname)
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
	if (getClientNick(params[0]))
	{
		client->addOutBuffer(std::string("433 * " + params[0] + " :Nickname is already in use\r\n"));
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
	client->addOutBuffer(std::string("001 " + nickname + " :Welcome to the Internet Relay Network " + nickname + "!" + username + "@" + realname + "\r\n"));
	// Send RPL_YOURHOST: 002
	client->addOutBuffer(std::string("002 " + nickname + " :Your host is " + servername + ", running version 1.0\r\n"));
	// Send RPL_CREATED: 003
	client->addOutBuffer(std::string("003 " + nickname + " :This server was created POR GERONIMA" + "\r\n"));
	// Send RPL_MYINFO: 004
	client->addOutBuffer(std::string("004 " + nickname + " " + servername + " 1.0 o o\r\n"));
	client->nextStatus();
}

/*void Server::addClient(Client *newClient)
{
	_clients.push_back(newClient);
}*/

void Server::_cmdPingSend(Client *client, std::vector<std::string> params)
{
	if (params.size() < 1)
	{
		client->addOutBuffer(std::string("409 * :No origin specified\r\n"));
		return;
	}
	client->addOutBuffer(std::string("PONG " + params[0] + "\r\n"));
	return;
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

void Server::_cmdMode(Client *client, std::vector<std::string> params)
{
	if (client->getStatus() != REG)
	{
		client->addOutBuffer(std::string("451 * :You have not registered\r\n"));
		return;
	}

	if (params.size() < 1)
	{
		client->addOutBuffer(std::string("461 " + client->getNickName() + " MODE :Not enough parameters\r\n"));
		return;
	}

	/*std::string target = params[0];
	if (target[0] == '#' || target[0] == '&') // Channel mode
	{
		_cmdChannelMode(client, params);
	}
	else // User mode
	{
		client->addOutBuffer(std::string("502 " + client->getNickName() + " :Cannot change mode for other users\r\n"));
	}*/
}

/*void _cmdChannelMode(Client *client, std::vector<std::string> params)
{
	(void)params;
	if (client->getStatus() != REG)
	{
		client->addOutBuffer(std::string("451 * :You have not registered\r\n"));
		return;
	}
}*/