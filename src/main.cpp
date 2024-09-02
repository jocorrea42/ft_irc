/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apodader <apodader@student.42barcel>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/05/23 08:30:22 by fili              #+#    #+#             */
/*   Updated: 2024/09/02 10:36:18 by apodader         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage:  ./ircserv <port> <password>" << std::endl;
        return (0);
    }
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