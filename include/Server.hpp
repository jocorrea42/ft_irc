/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apodader <apodader@student.42barcel>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 09:33:21 by fili              #+#    #+#             */
/*   Updated: 2024/07/22 16:04:02 by apodader         ###   ########.fr       */
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
# include <cerrno>

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

	void	_ClearClient(int fd); //-> clear client
	void	_nickAutentication(Client *client, const std::vector<std::string> &params);
	void	_userAutentication(Client *client, const std::vector<std::string> &params);
	void	_passAutentication(Client *client, const std::vector<std::string> &params);
	void	_cmdPingSend(Client *client, const std::vector<std::string> &params);
	void	_cmdCap(Client *client, const std::vector<std::string> &params);
	void	_cmdQuit(Client *client, const std::vector<std::string> &params);
	bool	_nickNameOk(const std::string &nickname);
	void	_cmdMode(Client *client, const std::vector<std::string> &params);
	void	_cmdChannelMode(Client *client, std::vector<std::string> params);
	void	_cmdJoin(Client *client, const std::vector<std::string> &params);
	void	_cmdMsg(Client *client, const std::vector<std::string> &params);
	void	_cmdKick(Client *client, std::vector<std::string> params);
	void	_cmdInvite(Client *client, std::vector<std::string> params);
	void	_cmdTopic(Client *client, const std::vector<std::string> &params);
	void 	_broadcastAllServer(const std::string &message);
	void	_cmdPrivmsg(Client* client, const std::vector<std::string> &params);

public:
	Server(); //-> default constructor
	Server(int port, std::string password);
	~Server();
	Server &operator=(Server const &other);
	int			getFd();
	int 		getPort();
	static bool isBotfull; // para el bonus
	std::string getPassword();
	Client 		*getClient(const int &fd);
	Client 		*getClientNick(const std::string &nick);
	int			getClientFd(const std::string &nick);
	void 		setFd(const int &fd);
	void 		setPort(const int &port);
	void 		setPassword(const std::string &pass);
	void 		addClient(const Client &newClient);
	void		addChannel(Client *client, const std::vector<std::string> &params);
//	void 		addFds(pollfd newFd);
	void 		setUsername(const std::string &username, const int &fd);
	void 		setNickname(const std::string &cmd, const int &fd);
	void		ServerStart();			 //-> server initialization
	void 		AcceptNewClient();		 //-> accept new client
	void 		ReceiveNewData(int fd); //-> receive new data from a registered client
	void		addPollfd(int fd); //-> agrego un elemento al vector poll de un cliente o el propio server 
	static void SignalHandler(int signum); //-> signal handler
	Channel		*getChannel(const std::string &name);
	void 		RemoveClient(int fd);
	void 		RemoveChannel(const std::string &name);
	void 		RemoveFds(int fd);
	void		RmChannels(int fd);
	void 		_CloseFds();		   //-> close file descriptors	
	void 		printParam(std::vector<std::string> params);
};
#endif