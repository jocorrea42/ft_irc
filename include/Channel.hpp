/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apodader <apodader@student.42barcel>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/23 08:05:05 by fili              #+#    #+#             */
/*   Updated: 2024/06/26 02:07:25 by apodader         ###   ########.fr       */
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
    std::vector<Client*> _clients;
    std::vector<Client*> _admins;

public:
    Channel(){};
    ~Channel(){};
    Channel(Channel const &other){*this = other;}
    Channel &operator=(Channel const &other);
    Channel(std::string name): _name(name){}
    void SetPassword(std::string password){this->_password = password;}
    void SetName(std::string name){this->_name = name;}
    std::string GetPassword(){return this->_password;}
	std::string GetName(){return this->_name;}
    void add_client(Client *newClient){this->_clients.push_back(newClient);}
    void add_admin(Client *newClient){this->_admins.push_back(newClient);}
    void remove_client(int fd);
    void remove_admin(int fd);
    bool change_clientToAdmin(std::string &nick);
    bool change_adminToClient(std::string &nick);
    void sendTo_all(std::string rpl1);
	void sendTo_all(std::string rpl1, int fd);

};
#endif