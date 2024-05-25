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

void Server::SerSocket()
{
	struct sockaddr_in add;
	struct pollfd NewPoll;
	add.sin_family = AF_INET;		  //-> set the address family to ipv4
	add.sin_port = htons(this->Port); //-> convert the port to network byte order (big endian)
	add.sin_addr.s_addr = INADDR_ANY; //-> set the address to any local machine address

	SerSocketFd = socket(AF_INET, SOCK_STREAM, 0); //-> create the server socket
	if (SerSocketFd == -1)						   //-> check if the socket is created
		throw(std::runtime_error("faild to create socket"));

	int en = 1;
	if (setsockopt(SerSocketFd, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1) //-> set the socket option (SO_REUSEADDR) to reuse the address
		throw(std::runtime_error("faild to set option (SO_REUSEADDR) on socket"));
	if (fcntl(SerSocketFd, F_SETFL, O_NONBLOCK) == -1) //-> set the socket option (O_NONBLOCK) for non-blocking socket
		throw(std::runtime_error("faild to set option (O_NONBLOCK) on socket"));
	if (bind(SerSocketFd, (struct sockaddr *)&add, sizeof(add)) == -1) //-> bind the socket to the address
		throw(std::runtime_error("faild to bind socket"));
	if (listen(SerSocketFd, SOMAXCONN) == -1) //-> listen for incoming connections and making the socket a passive socket
		throw(std::runtime_error("listen() faild"));

	NewPoll.fd = SerSocketFd; //-> add the server socket to the pollfd
	NewPoll.events = POLLIN;  //-> set the event to POLLIN for reading data
	NewPoll.revents = 0;	  //-> set the revents to 0
	fds.push_back(NewPoll);	  //-> add the server socket to the pollfd
}

void Server::ServerInit()
{
	this->Port = 4444;
	SerSocket(); //-> create the server socket

	std::cout << "Server <" << SerSocketFd << "> Connected, and is running!!!!!!!!" << std::endl;
	std::cout << "Waiting to accept a connection...\n";
	while (this->Signal == false)									  //-> run the server until the signal is received
	{																  // poll(fdsarray, fdsarraysize, time) el time en -1 bloquea hasta que exita evento en el poll
		if ((poll(&fds[0], fds.size(), -1) == -1) && Signal == false) // codigo para ver si ocurrio un evento
			throw(std::runtime_error("poll() fail, el vento salio mal"));
		for (int i = 0; i < fds.size(); i++) // miero todos los fds en el poll vector
		{
			if (fds[i].revents & POLLIN) // si el and es uno es por que revents es POLLIN o sea hay entrada para ser leida
			{
				if (fds[i].fd = SerSocketFd) // en este caso es una peticion al server de un cliente
					AcceptNewClient();
				else // en este caso es un mensaje de algun cliente
					ReceiveNewData(fds[i].fd);
			}
		}
	}
	CloseFds();
}

void Server::CloseFds()
{
	for (size_t i = 0; i < clients.size(); i++)
	{ //-> close all the clients
		std::cout << "Client <" << clients[i].getFd() << "> Disconnected" << std::endl;
		close(clients[i].getFd());
	}
	if (SerSocketFd != -1)
	{ //-> close the server socket
		std::cout << "Server <" << SerSocketFd << "> Disconnected" << std::endl;
		close(SerSocketFd);
	}
}

bool Server::Signal = false; //-> initialize the static boolean

void Server::SignalHandler(int signum)
{
	(void)signum;
	std::cout << std::endl
			  << "Signal Received!" << std::endl;
	Server::Signal = true; //-> set the static boolean to true to stop the server
}

void Server::AcceptNewClient()
{
	Client cli; //-> create a new client
	struct sockaddr_in cliadd;
	struct pollfd NewPoll;
	socklen_t len = sizeof(cliadd);

	int incofd = accept(SerSocketFd, (sockaddr *)&(cliadd), &len); //-> accept the new client
	if (incofd == -1)
	{
		std::cout << "accept() failed" << std::endl;
		return;
	}

	if (fcntl(incofd, F_SETFL, O_NONBLOCK) == -1) //-> set the socket option (O_NONBLOCK) for non-blocking socket
	{
		std::cout << "fcntl() failed" << std::endl;
		return;
	}

	NewPoll.fd = incofd;	 //-> add the client socket to the pollfd
	NewPoll.events = POLLIN; //-> set the event to POLLIN for reading data
	NewPoll.revents = 0;	 //-> set the revents to 0

	cli.setFd(incofd);						 //-> set the client file descriptor
	cli.setIp(inet_ntoa((cliadd.sin_addr))); //-> convert the ip address to string and set it
	clients.push_back(cli);					 //-> add the client to the vector of clients
	fds.push_back(NewPoll);					 //-> add the client socket to the pollfd

	std::cout << "Client <" << incofd << "> Connected" << std::endl;
}

void Server::ReceiveNewData(int fd)
{
	char buff[1024]; //-> buffer for the received data
	// memset(buff, 0, sizeof(buff)); //-> clear the buffer

	ssize_t bytes = recv(fd, buff, sizeof(buff) - 1, 0); //-> receive the data

	if (bytes <= 0)
	{ //-> check if the client disconnected
		std::cout << "Client <" << fd << "> Disconnected" << std::endl;
		ClearClients(fd); //-> clear the client
		close(fd);		  //-> close the client socket
	}

	else
	{ //-> print the received data
		buff[bytes] = '\0';
		std::cout << "Client <" << fd << "> Data: " << buff;
		// here you can add your code to process the received data: parse, check, authenticate, handle the command, etc...
	}
}

void Server::ClearClients(int fd)
{ //-> clear the clients
	for (size_t i = 0; i < fds.size(); i++)
	{ //-> remove the client from the pollfd
		if (fds[i].fd == fd)
		{
			fds.erase(fds.begin() + i);
			break;
		}
	}
	for (size_t i = 0; i < clients.size(); i++)
	{ //-> remove the client from the vector of clients
		if (clients[i].getFd() == fd)
		{
			clients.erase(clients.begin() + i);
			break;
		}
	}
}
