/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Bot Dev <bot@42.fr>                          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/02 10:00:00 by Bot Dev           #+#    #+#             */
/*   Updated: 2026/05/02 10:00:00 by Bot Dev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BOT_HPP
# define BOT_HPP

# include <string>
# include <vector>
# include <map>

class Server;
class Channel;
class Client;

class Bot
{
	public:

		Bot(const std::string& name = "BotMaster");
		~Bot();

		// Inicialización y control
		void setServer(Server* server);
		void onServerStart();
		void onChannelCreated(Channel* channel);
		void onMessageReceived(Channel* channel, const std::string& sender, const std::string& message);

		// Enviar mensajes
		void sendMessageToChannel(Channel* channel, const std::string& message);
		void sendPrivateMessage(const std::string& clientNick, const std::string& message);

		// Obtener info
		const std::string& getName() const;
		bool isActive() const;

	private:

		std::string	_name;
		Server*		_server;
		bool		_active;

		// Procesamiento de comandos
		bool _processCommand(Channel* channel, const std::string& sender, const std::string& message);
		void _handleHelpCommand(Channel* channel);
		void _handleInfoCommand(Channel* channel);
		void _handleListCommand(Channel* channel);
		void _handleWelcomeCommand(Channel* channel);
	void _handlePingCommand(Channel* channel, const std::string& sender);	void _handleEchoCommand(Channel* channel, const std::string& args);

	// Utilidades
	bool _isCommand(const std::string& message) const;
	std::string _extractCommand(const std::string& message) const;
	std::string _extractArgs(const std::string& message) const;		std::vector<std::string> _splitString(const std::string& str, char delimiter) const;
};

#endif
