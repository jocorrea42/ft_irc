/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fili <fili@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/23 08:05:05 by fili              #+#    #+#             */
/*   Updated: 2024/09/02 10:20:51 by fili             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include "Client.hpp"

# define ERR_OPNEEDED "482 * " + client->getNickName() + " :You're not channel operator\r\n"
# define ERR_NOCHANEL "403 " + client->getNickName() + params[0] + ":No such channel\r\n"
# define ERR_PARAM461 "461 " + client->getNickName() + " INVITE :Not enough parameters\r\n"
# define ERR_CHANN422 "442 * " + params[0] + " :You're not on that channel\r\n"
# define ERR_TOPIC331 "331 " + params[0] + " :No topic is set\r\n"
# define ERR_NICKN401 "401 " + params[1] + " :No such nick/channel: " + params[0] + "\r\n"
# define ERR_INCHA443 "443 " + params[0] + " " + params[1] + " :is already on channel\r\n"
# define MSG_TOPIC332 "332 " + params[0] + " :" + channel->getTopic() + "\r\n"

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
	std::vector<std::string> _invited;
    std::vector<std::string> _clients;
    std::vector<std::string> _admins;

public:
    Channel();
    ~Channel();
    Channel(Channel const &other){*this = other;}
    Channel &operator=(Channel const &other);
    Channel(std::string name, Client *client);
	Channel(std::string name, std::string password, Client *client);
    void setPassword(std::string password){this->_password = password;}
    void SetName(std::string name){this->_name = name;}
	void setInvOnly(bool set){_invOnly = set;};
	void setTopicLock(bool set){_topicLock = set;};
	void setLimit(int n);
	std::string getTopic();
    std::string getPassword(){return this->_password;}
	std::string getName(){return this->_name;}
	bool isClient(Client *fd);
	bool isInvited(std::string nick);
	void invite(std::string const &nick);
	bool isAdmin(std::string nick);
	bool isInvOnly();
	bool isTopicLocked();
	bool isFull();
    void addClient(Client *client);
    void addAdmin(Client *client);
    bool removeClient(std::string nick);
    void removeAdmin(std::string nick);
	void removeInvited(std::string const &nick);
	void setTopic(const std::string &newTopic);
    void giveTakeAdmin(int fd, const std::string &nick, Client *client);
	std::vector<std::string>	getClients(){ return (this->_clients);}
	std::string			getMode();

};
#endif