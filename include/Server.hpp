/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apodader <apodader@student.42barcel>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 09:33:21 by fili              #+#    #+#             */
/*   Updated: 2024/06/26 02:07:51 by apodader         ###   ########.fr       */
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
# include <iostream>
# include <sstream>
# include <cstdlib>

class Client;
class Channel;

class Server //-> class for server
{
private:
	int _port;
	int _fd;//esto no lo necesito por que lo guardo en polls
	static bool _Signal; 
	std::string _pass;			 // el subject pude un pass
	std::vector<Client> _clients; 
	std::vector<Channel> _channels;	// al igual que clientes
	std::vector<struct pollfd> _fds; // son los fd de los clientes, con el objetivo de monitorear con poll
	unsigned short	_polls_size; 
	struct sockaddr_in _add;		  // estructura de datos relacionada con la configuracion del socket
	struct sockaddr_in _clientadd; // lo mismo pero para un nuevo cliente conectado

	void 		_ClearClient(int fd); //-> clear client
	void		_nickAutentication(Client *client, std::vector<std::string> params);
	void		_userAutentication(Client *client,std::vector<std::string> params);
	void		_passAutentication(Client *client,std::vector<std::string> params);
	void		_cmdPingSend(Client *client, std::vector<std::string> params);

public:
	Server(); //-> default constructor
	Server(int port, std::string password);
	~Server();
	Server &operator=(Server const &other);
	int			getFd();
	int 		getPort();
	static bool isBotfull; // para el bonus
	std::string getPassword();
	Client 		*getClient(int fd);
	Client 		*getClientNick(std::string nick);
	void 		setFd(int fd);
	void 		setPort(int port);
	void 		setPassword(std::string pass);
	void 		addClient(Client newClient);
//	void 		addFds(pollfd newFd);
	void 		setUsername(std::string &username, int fd);
	void 		setNickname(std::string cmd, int fd);
	void		ServerStart();			 //-> server initialization
	void 		AcceptNewClient();		 //-> accept new client
	void 		ReceiveNewData(int fd); //-> receive new data from a registered client
	void		addPollfd(int fd); //-> agrego un elemento al vector poll de un cliente o el propio server 
	static void SignalHandler(int signum); //-> signal handler
	Client 		*getClien(int fd);
	
	void 		RemoveClient(int fd);
	void 		RemoveChannel(std::string name);
	void 		RemoveFds(int fd);
	void		RmChannels(int fd);
	void 		_CloseFds();		   //-> close file descriptors	
	void 		printParam(std::vector<std::string> params);
};
#endif