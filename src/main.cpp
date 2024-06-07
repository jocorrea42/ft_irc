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

int main(int argc, char **argv)
{
    Server ircserv;

    if (argc != 3)
    {
        std::cerr << "Usage:  ./ircserv <port> <password>" << std::endl;
        return (0);
    }
    else
    {
        try
        {
            signal(SIGINT, ircserv.SignalHandler);  //-> catch the signal (ctrl + c)
            signal(SIGQUIT, ircserv.SignalHandler); //-> catch the signal (ctrl + \)
            ircserv.setPassword(std::string(argv[2]));
            ircserv.ServerStart(atoi(argv[1]));
        }
        catch (const std::exception &e)
        {
            ircserv.CloseFds();
            std::cerr << e.what() << std::endl;
        }
        std::cout << "The Server Closed!" << std::endl;
    }
    return (0);
}