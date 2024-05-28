/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fili <fili@student.42.fr>                  #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024-05-22 09:33:21 by fili              #+#    #+#             */
/*   Updated: 2024-05-22 09:33:21 by fili             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <vector>
# include <sys/socket.h>
# include <sys/types.h>
# include <netinet/in.h>
# include <fcntl.h>
# include <unistd.h>
# include <poll.h>
# include <csignal>
# include <arpa/inet.h> //-> for inet_ntoa()
# include <string.h>
# include "Client.hpp"
# include "Channel.hpp"
class Client;
class Channel;

class Server //-> class for server
{
private:
	int _port;
	int _fd;
	static bool _Signal; 
	std::string _pass;			 // el subject pude un pass
	std::vector<Client> _clients; // relacion de agregacion
	// un servidor puede tener muchos clientes, mientras un cliente pertenece a un servidor
	// debido a que el cliente no se construye desde el server esta relacion es de agregacion
	// y no de composicion dado que el server puede no tener clientes, pudiera ser una lista u otro tipo de dato
	//std::vector<Channel> _channels;	// al igual que clientes
	std::vector<struct pollfd> _fds; // son los fd de los clientes, con el objetivo de monitorear con poll
	// la aparicion de algun evento o cambio de estado de alguno de ellos, puede ser alguna escritura, o simplemente
	//  un evento determinado se mantiene la escucha por un tiempo en cada fd
	struct sockaddr_in _add;		  // estructura de datos relacionada con la configuracion del socket
	struct sockaddr_in _clientadd; // lo mismo pero para un nuevo cliente conectado
public:
	Server(); //-> default constructor
	Server(int port, std::string password);
	~Server(){};
	Server &operator=(Server const &other);
	// setters y getters en funcion de necesidades
	int			getFd();
	int 		getPort();
	static bool isBotfull; // para el bonus
	std::string getPassword();
	Client 		*getClient(int fd);
	Client 		*getClientNick(std::string nick);
	//Channel *getChannel(std::string name);
	void 		setFd(int fd);
	void 		SetPort(int port);
	void 		setPassword(std::string pass);
	void 		addClient(Client newClient);
	//void addChannel(Channel newChannel);
	void 		addFds(pollfd newFd);
	void 		setUsername(std::string &username, int fd);
	void 		setNickname(std::string cmd, int fd);

	void		ServerStart(int port);			 //-> server initialization
	void 		ServerSocketCreate();			 //-> server socket creation
	void 		AcceptNewClient();		 //-> accept new client
	void 		ReceiveNewData(int fd); //-> receive new data from a registered client
	void		addPollfd(int fd); //-> agrego un elemento al vector poll de un cliente o el propio server 
	static void SignalHandler(int signum); //-> signal handler

	void CloseFds();		   //-> close file descriptors
	void ClearClients(int fd); //-> clear clients
};
#endif