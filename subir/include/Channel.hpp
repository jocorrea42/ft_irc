/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fili <fili@student.42.fr>                  #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024-05-23 08:05:05 by fili              #+#    #+#             */
/*   Updated: 2024-05-23 08:05:05 by fili             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Client.hpp"
#include "Server.hpp"

class Client;

class Channel
{
private:
    std::string name;
    std::string password;
    std::vector<Client> clients;
    std::vector<Client> admins;

public:
    Channel();
    ~Channel();
    Channel(Channel const &src);
    Channel &operator=(Channel const &src);
    void SetPassword(std::string password);
    void SetName(std::string name);
    std::string GetPassword();
	std::string GetName();
    void add_client(Client newClient);
    void add_admin(Client newClient);
    void remove_client(int fd);
    void remove_admin(int fd);
    bool change_clientToAdmin(std::string &nick);
    bool change_adminToClient(std::string &nick);
    void sendTo_all(std::string rpl1);
	void sendTo_all(std::string rpl1, int fd);

};
#endif