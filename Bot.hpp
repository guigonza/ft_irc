/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Guille <Guille@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 12:00:00 by Guille            #+#    #+#             */
/*   Updated: 2026/04/28 12:00:00 by Guille           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#ifndef BOT_HPP
# define BOT_HPP

# include <string>
# include <vector>
# include <map>
# include <ctime>
# include <sys/socket.h>

class Bot
{
	public:

		Bot(const std::string& host, int port, const std::string& password);
		~Bot();

		void run();

	private:

		// OCF — prohibido copiar
		Bot();
		Bot(const Bot&);
		Bot& operator=(const Bot&);

		// Conexión
		void _connect();
		void _authenticate();
		void _disconnect();

		// Bucle principal
		void _handleMessage(const std::string& line);
		std::string _receive();
		void _send(const std::string& msg);

		// Parseo de comandos
		void _processCommand(const std::string& channel, 
							const std::string& user,
							const std::string& command);

		// Comandos del bot
		void _cmdHelp(const std::string& channel);
		void _cmdEcho(const std::string& channel, const std::string& args);
		void _cmdInfo(const std::string& channel);
		void _cmdContador(const std::string& channel, const std::string& args);
		void _cmdHora(const std::string& channel);
		void _cmdUsuarios(const std::string& channel);
		void _cmdPing(const std::string& channel);

		// Helpers
		void _updateChannels(const std::string& line);

		// Atributos
		std::string                    _host;
		int                            _port;
		std::string                    _password;
		std::string                    _nick;
		int                            _fd;
		std::string                    _inbuf;
		int                            _commandCount;
		time_t                         _startTime;
		std::vector<std::string>       _joinedChannels;
		bool                           _authenticated;
};

#endif
