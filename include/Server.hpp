/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jocorrea <jocorrea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/22 09:33:21 by fili              #+#    #+#             */
/*   Updated: 2024/09/22 17:00:16 by jocorrea         ###   ########.fr       */
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
#include <arpa/inet.h>
#include <string.h>
#include "Client.hpp"
#include "Channel.hpp"
#include <sstream>
#include <cstdlib>
#include <cerrno>
#include <ctime>
#include <iomanip>
#include <fstream>

class Client;
class Channel;

class Server
{
private:
	int _port;
	int _fd;
	static bool _Signal;
	std::string _pass;
	std::vector<Client> _clients;
	std::vector<Channel> _channels;
	std::vector<struct pollfd> _fds;
	unsigned short _polls_size;
	struct sockaddr_in _add;

	void _ClearClient(int fd);
	void _nickAutentication(Client *client, const std::vector<std::string> &params);
	void _userAutentication(Client *client, const std::vector<std::string> &params);
	void _passAutentication(Client *client, const std::vector<std::string> &params);
	void _cmdPingSend(Client *client, const std::vector<std::string> &params);
	void _cmdCap(Client *client, const std::vector<std::string> &params);
	void _cmdQuit(Client *client, const std::vector<std::string> &params);
	bool _nickNameOk(const std::string &nickname);
	void _cmdMode(Client *client, const std::vector<std::string> &params);
	void _cmdChannelMode(Client *client, std::vector<std::string> params);
	void _cmdJoin(Client *client, const std::vector<std::string> &params);
	void _cmdKick(Client *client, std::vector<std::string> params);
	void _cmdInvite(Client *client, std::vector<std::string> params);
	void _cmdTopic(Client *client, const std::vector<std::string> &params);
	void _broadcastAllServer(const std::string &message);
	void _cmdPrivmsg(Client *client, std::vector<std::string> params);
	void _disconnectClient(Client *client, std::string msg, int mode);
	void _broadcastClientChannel(Channel *channel, std::string msg, int fd);
	std::vector<std::string> _splitStr(const std::string &str, char delimiter);
	void _addClient(int inConectionFd, struct sockaddr_in  clientadd);
	void _bot(Client *client, std::vector<std::string> params);
	void _botHour(Client *client, std::vector<std::string> params);	
	void _botRandom(Client *client, std::vector<std::string> params);
	void _cmdWho(Client* client, std::vector<std::string> params);
public:
	Server();
	Server(int port, std::string password);
	~Server();
	Server &operator=(Server const &other);
	int const &getFd();
	int const &getPort();
	static bool isBotfull;
	std::string getPassword();
	Client *getClient(const int &fd);
	Client *getClient(const std::string &nick);
	Client *getClientNick(const std::string &nick);
	int getClientFd(const std::string &nick);
	void setFd(const int &fd);
	void setPort(const int &port);
	void setPassword(const std::string &pass);
	void addChannel(Client *client, const std::vector<std::string> &params);
	void changeNickname(Client *client, const std::string &newNick);
	void ServerStart();
	void AcceptNewClient();
	void ReceiveNewData(int fd);
	void addPollfd(int fd);
	static void SignalHandler(int signum);
	Channel *getChannel(const std::string &name);
	void RemoveChannel(const std::string &name);
	void _CloseFds();
};
#endif