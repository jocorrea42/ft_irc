/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apodader <apodader@student.42barcel>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/23 08:05:05 by fili              #+#    #+#             */
/*   Updated: 2024/08/26 12:39:40 by apodader         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include "Client.hpp"

# define ERR_OPNEEDED "482 " + client->getNickName() + " :You're not channel operator\r\n"
# define ERR_NOCHANEL "403 " + client->getNickName() + params[0] + ":No such channel\r\n"

class Client;

class Channel
{
private:
    std::string _name;
    std::string _password;
	std::string _topic;
	bool		_invOnly;
	bool		_topicLock;
	int			_limit;
	std::vector<int>	_invited;
    std::vector<int> _clients;
    std::vector<int> _admins;

public:
    Channel();
    ~Channel();
    Channel(Channel const &other){*this = other;}
    Channel &operator=(Channel const &other);
    Channel(std::string name, Client *client);
	Channel(std::string name, std::string password, Client *client);
    void SetPassword(std::string password){this->_password = password;}
    void SetName(std::string name){this->_name = name;}
	void setInvOnly();
	void unsetInvOnly();
	void setTopicLock();
	void unsetTopicLock();
	void setLimit(int n);
	std::string getTopic();
    std::string GetPassword(){return this->_password;}
	std::string GetName(){return this->_name;}
	bool isClient(Client *fd);
	bool isInvited(int fd);
	bool invite(int fd);
	bool isAdmin(int fd);
	bool isInvOnly();
	bool isTopicLocked();
	bool isFull();
    void add_client(Client *client);
    void add_admin(Client *client);
    bool remove_client(int fd);
    void remove_admin(int fd);
	void setTopic(const std::string &newTopic);
    void GiveTakeAdmin(int fd, const std::string &nick, Client *client);
	std::vector<int>	getClients(){ return (this->_clients);}
   // void sendToAll(std::string msg, Client *fd);

};
#endif