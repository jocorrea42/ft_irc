/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fili <fili@student.42.fr>                  #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024-05-27 08:20:36 by fili              #+#    #+#             */
/*   Updated: 2024-05-27 08:20:36 by fili             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <csignal>
#include <arpa/inet.h> //-> for inet_ntoa()

class Server //-> class for server
{
private:
	int port;
	int fd;
	std::string pass;			 // el subject pude un pass
//	std::vector<Client> clients; // relacion de agregacion
	// un servidor puede tener muchos clientes, mientras un cliente pertenece a un servidor
	// debido a que el cliente no se construye desde el server esta relacion es de agregacion
	// y no de composicion dado que el server puede no tener clientes, pudiera ser una lista u otro tipo de dato
//	std::vector<Channel> channels;	// al igual que clientes
	std::vector<struct pollfd> fds; // son los fd de los clientes, con el objetivo de monitorear con poll
	// la aparicion de algun evento o cambio de estado de alguno de ellos, puede ser alguna escritura, o simplemente
	//  un evento determinado se mantiene la escucha por un tiempo en cada fd
	struct sockaddr_in add;		  // estructura de datos relacionada con la configuracion del socket
	struct sockaddr_in clientadd; // lo mismo pero para un nuevo cliente conectado
	struct pollfd newClient;	  // lo mismo, estas tres estructuras se utilizan para la creacion y control de
								  // los nuevos clientes conectados
public:
	Server() { fd = -1; } //-> default constructor
	~Server(){};
	Server &operator=(Server const &other);
	// setters y getters en funcion de necesidades
	int GetFd();
	int GetPort();
	static bool isBotfull; // para el bonus
	std::string GetPassword();
//	Client *GetClient(int fd);
//	Client *GetClientNick(std::string nick);
//	Channel *GetChannel(std::string name);
	void SetFd(int fd);
	void SetPort(int port);
	void SetPassword(std::string pass);
//	void AddClient(Client newClient);
//	void AddChannel(Channel newChannel);
	void AddFds(pollfd newFd);
	void set_username(std::string &username, int fd);
	void set_nickname(std::string cmd, int fd);

	void ServerInit();			 //-> server initialization
	void SerSocket();			 //-> server socket creation
	void AcceptNewClient();		 //-> accept new client
	void ReceiveNewData(int fd); //-> receive new data from a registered client

	static void SignalHandler(int signum); //-> signal handler

	void CloseFds();		   //-> close file descriptors
	void ClearClients(int fd); //-> clear clients
};
#endif
