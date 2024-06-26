/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apodader <apodader@student.42barcel>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/23 08:30:22 by fili              #+#    #+#             */
/*   Updated: 2024/06/26 02:06:56 by apodader         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

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
           
            ircserv = Server(atoi(argv[1]), std::string(argv[2]));
            ircserv.setPassword("ok");
            ircserv.ServerStart();
        }
        catch (const std::exception &e)
        {
            ircserv._CloseFds();
            std::cerr << e.what() << std::endl;
        }
        std::cout << "The Server Closed!" << std::endl;
    }
    return (0);
}