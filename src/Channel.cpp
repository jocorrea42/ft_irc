/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fili <fili@student.42.fr>                  #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024-05-23 08:12:02 by fili              #+#    #+#             */
/*   Updated: 2024-05-23 08:12:02 by fili             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Channel.hpp"

// Channel();
//     ~Channel();
//     Channel(Channel const &src);
//     Channel &operator=(Channel const &src);
//     void SetPassword(std::string password);
//     void SetName(std::string name);
//     std::string GetPassword();
// 	std::string GetName();
//     void add_client(Client newClient);
//     void add_admin(Client newClient);
//     void remove_client(int fd);
//     void remove_admin(int fd);
//     bool change_clientToAdmin(std::string &nick);
//     bool change_adminToClient(std::string &nick);
//     void sendTo_all(std::string rpl1);
// 	void sendTo_all(std::string rpl1, int fd);
Channel::Channel(Channel const &src){*this = src;}
Channel &Channel::operator=(Channel const &src)
{
    return *this;
}
Channel::~Channel(){}
