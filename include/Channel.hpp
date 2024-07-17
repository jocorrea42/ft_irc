/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apodader <apodader@student.42barcel>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/23 08:05:05 by fili              #+#    #+#             */
/*   Updated: 2024/07/17 17:13:44 by apodader         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include "Client.hpp"

class Client;

class Channel
{
private:
    std::string _name;
    std::string _password;
	std::string _topic;
	bool		_invOnly;
	std::vector<int>	_invited;
    std::vector<Client*> _clients;
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
    std::string GetPassword(){return this->_password;}
	std::string GetName(){return this->_name;}
	bool isClient(int fd);
	bool isInvited(int fd);
	bool invite(int fd);
	bool isAdmin(int fd);
	bool isInvOnly();
    void add_client(Client *client);
    void add_admin(Client *client);
    bool remove_client(int fd);
    void remove_admin(int nick);
	void setTopic(const std::string &newTopic);
    bool change_clientToAdmin(std::string &nick);
    bool change_adminToClient(std::string &nick);
    void sendToAll(std::string msg);

};
#endif