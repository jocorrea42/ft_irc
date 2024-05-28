/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fili <fili@student.42.fr>                  #+#  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024-05-23 08:30:22 by fili              #+#    #+#             */
/*   Updated: 2024-05-23 08:30:22 by fili             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"

int main()
{
    Server irc_server;
    try
    {
        signal(SIGINT, irc_server.SignalHandler); //-> catch the signal (ctrl + c)
		signal(SIGQUIT, irc_server.SignalHandler); //-> catch the signal (ctrl + \)
        irc_server.ServerStart(4444);
    }catch(const std::exception &e)
    {
        irc_server.CloseFds();
        std::cerr << e.what() << std::endl;
    }  
    std::cout << "The Server Closed!" << std::endl;
}