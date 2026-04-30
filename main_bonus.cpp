/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_bonus.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: carbon <carbon@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/23 10:11:06 by Guille            #+#    #+#             */
/*   Updated: 2026/04/28 21:24:27 by carbon           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Bot.hpp"
#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }

    // Validar puerto
    int port = std::atoi(argv[1]);
    if (port <= 0 || port > 65535)
    {
        std::cerr << "Error: invalid port" << std::endl;
        return 1;
    }

    // Validar password no vacío
    std::string password = argv[2];
    if (password.empty())
    {
        std::cerr << "Error: password cannot be empty" << std::endl;
        return 1;
    }

    try
    {
        Server server(port, password);

        // Iniciar bot en proceso hijo (fork)
        pid_t pid = fork();
        if (pid == 0)
        {
            // Proceso hijo: ejecuta el bot
            sleep(1); // Esperar a que el servidor esté listo
            try
            {
                Bot bot("localhost", port, password);
                bot.run();
            }
            catch (const std::exception& e)
            {
                std::cerr << "[Bot Error] " << e.what() << std::endl;
            }
            exit(0);
        }
        else if (pid > 0)
        {
            // Proceso padre: ejecuta el servidor
            std::cout << "[Server] Iniciando servidor en puerto " << port << std::endl;
            std::cout << "[Server] Bot iniciado en proceso " << pid << std::endl;
            
            // Ignorar SIGCHLD para que el proceso hijo se limpie automáticamente
            signal(SIGCHLD, SIG_IGN);
            
            server.run();
        }
        else
        {
            throw std::runtime_error("fork() failed");
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
