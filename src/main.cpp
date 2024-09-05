/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jocorrea <jocorrea@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/23 08:30:22 by fili              #+#    #+#             */
/*   Updated: 2024/09/05 17:56:25 by jocorrea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

int main(int argc, char **argv)
{
    int port = atoi(argv[1]);
    if (argc != 3)
        std::cerr << "Usage:  ./ircserv <port> <password>" << std::endl;
    else if (port < 1024 || port > 49151)
        std::cerr << "invalid port: port range 1024-49151" << std::endl;
    else
    {
        Server ircserv(atoi(argv[1]), std::string(argv[2]));
        try
        {
            signal(SIGINT, ircserv.SignalHandler);
            signal(SIGQUIT, ircserv.SignalHandler);
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