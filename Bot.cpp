/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Bot Dev <bot@42.fr>                          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/02 10:00:00 by Bot Dev           #+#    #+#             */
/*   Updated: 2026/05/02 10:00:00 by Bot Dev          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bot.hpp"
#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"
#include <iostream>
#include <sstream>
#include <ctime>

// ─────────────────────────────────────────────
// Constructor / Destructor
// ─────────────────────────────────────────────

Bot::Bot(const std::string& name)
	: _name(name), _server(NULL), _active(false)
{
	std::cout << "[Bot] Created bot '" << _name << "'" << std::endl;
}

Bot::~Bot()
{
	std::cout << "[Bot] Bot '" << _name << "' destroyed" << std::endl;
}

// ─────────────────────────────────────────────
// Inicialización
// ─────────────────────────────────────────────

void Bot::setServer(Server* server)
{
	_server = server;
	if (_server)
		_active = true;
}

void Bot::onServerStart()
{
	if (!_active)
		return;
	std::cout << "[Bot] Server started! Bot '" << _name << "' is online" << std::endl;
}

void Bot::onChannelCreated(Channel* channel)
{
	if (!_active || !channel)
		return;
	std::cout << "[Bot] New channel created: " << channel->getName() << std::endl;
	// Enviar mensaje de bienvenida del bot al nuevo canal
	sendMessageToChannel(channel, "Welcome to " + channel->getName() + "! I'm " + _name + ", type !help for commands.");
}

void Bot::onMessageReceived(Channel* channel, const std::string& sender, const std::string& message)
{
	if (!_active || !channel)
		return;

	std::string normalized = message;
	while (!normalized.empty() && (normalized[0] == ' ' || normalized[0] == '\t'))
		normalized.erase(0, 1);

	std::cout << "[Bot] Received message in " << channel->getName() 
			  << " from " << sender << ": '" << message << "'" << std::endl;

	// Comprobar si es un comando del bot (empieza con '!')
	if (_isCommand(normalized))
	{
		std::cout << "[Bot] Detected command in message" << std::endl;
		if (_processCommand(channel, sender, normalized))
			return; // Se procesó el comando
	}
}

// ─────────────────────────────────────────────
// Enviar mensajes
// ─────────────────────────────────────────────

void Bot::sendMessageToChannel(Channel* channel, const std::string& message)
{
	if (!channel)
	{
		std::cout << "[Bot] ERROR: channel is NULL" << std::endl;
		return;
	}

	if (!_server || !_active)
	{
		std::cout << "[Bot] ERROR: server or bot not active" << std::endl;
		return;
	}

	std::cout << "[Bot] About to broadcast to " << channel->getName() 
			  << " (members: " << channel->memberCount() << ")" << std::endl;

	// Formato IRC: :bot_name!bot@localhost PRIVMSG #channel :message
	std::string botPrefix = _name + "!bot@localhost";
	std::string ircMsg = ":" + botPrefix + " PRIVMSG " + channel->getName() + " :" + message + "\r\n";

	std::cout << "[Bot] Message to send: " << ircMsg << std::endl;

	// Usar el broadcast del canal para enviar a todos
	channel->broadcast(ircMsg);

	std::cout << "[Bot] Broadcast complete" << std::endl;
}

void Bot::sendPrivateMessage(const std::string& clientNick, const std::string& message)
{
	if (!_server || !_active)
		return;

	std::cout << "[Bot] PM to " << clientNick << ": " << message << std::endl;

	// Formato IRC: :bot_name PRIVMSG nick :message
	std::string ircMsg = ":" + _name + " PRIVMSG " + clientNick + " :" + message + "\r\n";
	
	// Aquí se podría buscar el cliente y enviarle el mensaje
	// Por ahora solo se registra en consola
}

// ─────────────────────────────────────────────
// Getters
// ─────────────────────────────────────────────

const std::string& Bot::getName() const
{
	return _name;
}

bool Bot::isActive() const
{
	return _active;
}

// ─────────────────────────────────────────────
// Procesamiento de comandos
// ─────────────────────────────────────────────

bool Bot::_isCommand(const std::string& message) const
{
	return (!message.empty() && message[0] == '!');
}

std::string Bot::_extractCommand(const std::string& message) const
{
	if (!_isCommand(message))
		return "";

	size_t space = message.find(' ');
	if (space != std::string::npos)
		return message.substr(1, space - 1); // Quitar el '!'
	else
		return message.substr(1); // Toda la cadena sin el '!'
}

std::string Bot::_extractArgs(const std::string& message) const
{
	size_t space = message.find(' ');
	if (space != std::string::npos)
		return message.substr(space + 1);
	return "";
}

std::vector<std::string> Bot::_splitString(const std::string& str, char delimiter) const
{
	std::vector<std::string> tokens;
	std::stringstream ss(str);
	std::string token;

	while (std::getline(ss, token, delimiter))
		tokens.push_back(token);

	return tokens;
}

bool Bot::_processCommand(Channel* channel, const std::string& sender, const std::string& message)
{
	std::string cmd = _extractCommand(message);
	std::string args = _extractArgs(message);

	std::cout << "[Bot] Processing command '" << cmd << "' from " << sender << " in " << channel->getName() << std::endl;

	if (cmd == "help")
	{
		_handleHelpCommand(channel);
		return true;
	}
	else if (cmd == "info")
	{
		_handleInfoCommand(channel);
		return true;
	}
	else if (cmd == "list")
	{
		_handleListCommand(channel);
		return true;
	}
	else if (cmd == "welcome")
	{
		_handleWelcomeCommand(channel);
		return true;
	}
	else if (cmd == "ping")
	{
		_handlePingCommand(channel, sender);
		return true;
	}
	else if (cmd == "echo")
	{
		_handleEchoCommand(channel, args);
		return true;
	}

	return false; // Comando no reconocido
}

void Bot::_handleHelpCommand(Channel* channel)
{
	sendMessageToChannel(channel, "Available commands:");
	sendMessageToChannel(channel, "  !help     - Show this help message");
	sendMessageToChannel(channel, "  !info     - Show bot information");
	sendMessageToChannel(channel, "  !list     - List available commands");
	sendMessageToChannel(channel, "  !welcome  - Welcome message");
	sendMessageToChannel(channel, "  !ping     - Ping the bot");
	sendMessageToChannel(channel, "  !echo <text> - Echo back a message");
}

void Bot::_handleInfoCommand(Channel* channel)
{
	sendMessageToChannel(channel, "Bot: " + _name);
	sendMessageToChannel(channel, "Status: Online and ready");
	sendMessageToChannel(channel, "Version: 1.0");
}

void Bot::_handleListCommand(Channel* channel)
{
	sendMessageToChannel(channel, "Commands: help, info, list, welcome, ping, echo");
}

void Bot::_handleWelcomeCommand(Channel* channel)
{
	sendMessageToChannel(channel, "Welcome to the IRC server! Type !help for available commands.");
}

void Bot::_handlePingCommand(Channel* channel, const std::string& sender)
{
	sendMessageToChannel(channel, "Pong! " + sender);
}

void Bot::_handleEchoCommand(Channel* channel, const std::string& args)
{
	if (args.empty())
	{
		sendMessageToChannel(channel, "Usage: !echo <message>");
		return;
	}
	sendMessageToChannel(channel, "Echo: " + args);
}
