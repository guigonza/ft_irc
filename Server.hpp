/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Guille <Guille@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/23 10:11:03 by Guille            #+#    #+#             */
/*   Updated: 2026/03/23 11:42:23 by Guille           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include <vector>
# include <map>
# include <poll.h>


class Client;
class Channel;

class Server
{
	public:

		Server(int port, const std::string& password);
		~Server();

		void run();

	private:

		// OCF — prohibido copiar
		Server();
		Server(const Server&);
		Server& operator=(const Server&);

		// Setup
		void _setupSocket();

		// Event loop
		void _acceptClient();
		void _handleClient(int fd);
		void _disconnectClient(int fd);
		void _cleanEmptyChannels();

		// Command dispatch
		void _processLine(Client& c, const std::string& line);

		// Comandos de autenticación
		void _cmdPass(Client& c, const std::vector<std::string>& params);
		void _cmdNick(Client& c, const std::vector<std::string>& params);
		void _cmdUser(Client& c, const std::vector<std::string>& params,
					const std::string& trailing);

		// Comandos de canal
		void _cmdJoin   (Client& c, const std::vector<std::string>& params);
		void _cmdPart   (Client& c, const std::vector<std::string>& params,
						const std::string& trailing);
		void _cmdPrivmsg(Client& c, const std::vector<std::string>& params,
						const std::string& trailing);
		void _cmdQuit   (Client& c, const std::string& trailing);
		void _cmdPing   (Client& c, const std::vector<std::string>& params,
						const std::string& trailing);

		// Comandos de operador
		void _cmdKick  (Client& c, const std::vector<std::string>& params,
						const std::string& trailing);
		void _cmdInvite(Client& c, const std::vector<std::string>& params);
		void _cmdTopic (Client& c, const std::vector<std::string>& params,
						const std::string& trailing);
		void _cmdMode  (Client& c, const std::vector<std::string>& params);

		// Helpers
		void     _send       (int fd, const std::string& msg);
		void     _sendReply  (int fd, const std::string& code,
							const std::string& target, const std::string& msg);
		void     _sendWelcome(Client& c);
		Client*  _getClientByNick   (const std::string& nick);
		Channel* _getChannel        (const std::string& name);
		Channel* _getOrCreateChannel(const std::string& name);

		// Atributos
		int                             _port;
		std::string                     _password;
		std::string                     _name;
		int                             _serverFd;
		std::vector<struct pollfd>      _fds;
		std::map<int, Client*>          _clients;
		std::map<std::string, Channel*> _channels;
};

#endif