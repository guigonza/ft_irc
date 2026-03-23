/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Guille <Guille@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/23 10:10:58 by Guille            #+#    #+#             */
/*   Updated: 2026/03/23 11:42:33 by Guille           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"

# include <sys/socket.h>
# include <netinet/in.h>
# include <fcntl.h>
# include <unistd.h>
# include <cstring>
# include <iostream>
# include <sstream>
# include <cerrno>
# include <arpa/inet.h>

// ─────────────────────────────────────────────
// OCF
// ─────────────────────────────────────────────

Server::Server(int port, const std::string& password)
    : _port(port),
      _password(password),
      _name("ft_irc"),
      _serverFd(-1)
{
    _setupSocket();
}

Server::~Server()
{
    for (std::map<int, Client*>::iterator it = _clients.begin();
         it != _clients.end(); ++it)
    {
        close(it->first);
        delete it->second;
    }
    for (std::map<std::string, Channel*>::iterator it = _channels.begin();
         it != _channels.end(); ++it)
        delete it->second;
    if (_serverFd != -1)
        close(_serverFd);
}
// ─────────────────────────────────────────────
// Setup
// ─────────────────────────────────────────────

void Server::_setupSocket()
{
    _serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverFd == -1)
        throw std::runtime_error("socket() failed");

    int opt = 1;
    setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    fcntl(_serverFd, F_SETFL, O_NONBLOCK);

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(_port);

    if (bind(_serverFd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
        throw std::runtime_error("bind() failed");
    if (listen(_serverFd, SOMAXCONN) == -1)
        throw std::runtime_error("listen() failed");

    struct pollfd pfd;
    pfd.fd      = _serverFd;
    pfd.events  = POLLIN;
    pfd.revents = 0;
    _fds.push_back(pfd);

    std::cout << "[Server] Listening on port " << _port << std::endl;
}
// ─────────────────────────────────────────────
// Event loop
// ─────────────────────────────────────────────

void Server::run()
{
    while (true)
    {
        int ret = poll(_fds.data(), _fds.size(), -1);
        if (ret == -1)
        {
            if (errno == EINTR)
                break;
            throw std::runtime_error("poll() failed");
        }

        for (size_t i = 0; i < _fds.size(); i++)
        {
            if (_fds[i].revents & (POLLERR | POLLHUP))
            {
                if (_fds[i].fd != _serverFd)
                    _disconnectClient(_fds[i].fd);
                continue;
            }
            if (!(_fds[i].revents & POLLIN))
                continue;

            if (_fds[i].fd == _serverFd)
                _acceptClient();
            else
                _handleClient(_fds[i].fd);
        }
    }
}
// ─────────────────────────────────────────────
// Gestión de clientes
// ─────────────────────────────────────────────

void Server::_acceptClient()
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    int fd = accept(_serverFd, (struct sockaddr*)&addr, &len);
    if (fd == -1)
        return;

    fcntl(fd, F_SETFL, O_NONBLOCK);

    // Convertir la IP binaria a string "192.168.1.10"
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip));
    std::string hostname(ip);

    struct pollfd pfd;
    pfd.fd      = fd;
    pfd.events  = POLLIN;
    pfd.revents = 0;
    _fds.push_back(pfd);

    _clients[fd] = new Client(fd, hostname); // pasamos la IP
    std::cout << "[Server] New connection fd=" << fd
              << " ip=" << hostname << std::endl;
}

void Server::_handleClient(int fd)
{
    char buf[512];
    int bytes = recv(fd, buf, sizeof(buf) - 1, 0);

    if (bytes == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return; // no hay datos todavía, no es un error
        _disconnectClient(fd);
        return;
    }
    if (bytes == 0)
    {
        _disconnectClient(fd); // desconexión limpia del cliente
        return;
    }

    buf[bytes] = '\0';
    _clients[fd]->appendBuffer(buf, bytes);

    std::string line;
    while (_clients[fd]->getLine(line))
        _processLine(*_clients[fd], line);
}
void Server::_disconnectClient(int fd)
{
    if (_clients.find(fd) == _clients.end())
        return;

    Client* client = _clients[fd];

    // Notificar a los canales donde estaba
    std::string quitMsg = ":" + client->prefix()
                        + " QUIT :connection closed\r\n";
    for (std::map<std::string, Channel*>::iterator it = _channels.begin();
         it != _channels.end(); ++it)
    {
        if (it->second->hasMember(client))
        {
            it->second->broadcastExcept(quitMsg, client);
            it->second->removeMember(client);
        }
    }
    _cleanEmptyChannels();

    // Eliminar del array de poll
    for (size_t i = 0; i < _fds.size(); i++)
    {
        if (_fds[i].fd == fd)
        {
            _fds.erase(_fds.begin() + i);
            break;
        }
    }

    close(fd);
    delete client;
    _clients.erase(fd);
}

void Server::_cleanEmptyChannels()
{
    std::map<std::string, Channel*>::iterator it = _channels.begin();
    while (it != _channels.end())
    {
        if (it->second->memberCount() == 0)
        {
            delete it->second;
            it = _channels.erase(it);
        }
        else
            ++it;
    }
}
// ─────────────────────────────────────────────
// Command dispatch
// ─────────────────────────────────────────────

void Server::_processLine(Client& client, const std::string& line)
{
    if (line.empty())
        return;

    std::string              prefix;
    std::string              command;
    std::vector<std::string> params;
    std::string              trailing;

    std::istringstream ss(line);
    std::string        token;

    // ¿Tiene prefix?
    if (line[0] == ':')
        ss >> prefix;

    // Comando
    ss >> command;

    // Parámetros
    while (ss >> token)
    {
        if (token[0] == ':')
        {
            std::string rest;
            std::getline(ss, rest);
            trailing = token.substr(1) + rest;
            break;
        }
        params.push_back(token);
    }

    // Normalizar a mayúsculas — algunos clientes mandan en minúsculas
    for (size_t i = 0; i < command.size(); i++)
        command[i] = std::toupper(command[i]);

    std::cout << "[<<] fd=" << client.getFd()
              << " cmd=" << command << std::endl;

    // Comandos que no requieren registro
    if (command == "CAP")    return; // ignorar capability negotiation
    if (command == "PASS")   { _cmdPass(client, params);           return; }
    if (command == "NICK")   { _cmdNick(client, params);           return; }
    if (command == "USER")   { _cmdUser(client, params, trailing); return; }
    if (command == "QUIT")   { _cmdQuit(client, trailing);         return; }

    // Todo lo demás requiere estar registrado
    if (!client.isRegistered())
    {
        _sendReply(client.getFd(), "451", "*", "You have not registered");
        return;
    }

    if      (command == "JOIN")    _cmdJoin   (client, params);
    else if (command == "PART")    _cmdPart   (client, params, trailing);
    else if (command == "PRIVMSG") _cmdPrivmsg(client, params, trailing);
    else if (command == "KICK")    _cmdKick   (client, params, trailing);
    else if (command == "INVITE")  _cmdInvite (client, params);
    else if (command == "TOPIC")   _cmdTopic  (client, params, trailing);
    else if (command == "MODE")    _cmdMode   (client, params);
    else if (command == "PING")    _cmdPing   (client, params, trailing);
    else if (command == "WHO")     return; // algunos clientes lo piden, ignorar
    else
        _sendReply(client.getFd(), "421", client.getNick(),
                   command + " :Unknown command");
}
// ─────────────────────────────────────────────
// Autenticación
// ─────────────────────────────────────────────

void Server::_cmdPass(Client& client, const std::vector<std::string>& params)
{
    if (client.isRegistered())
    {
        _sendReply(client.getFd(), "462", client.getNick(),
                   "You may not reregister");
        return;
    }
    if (params.empty())
    {
        _sendReply(client.getFd(), "461", "*",
                   "PASS :Not enough parameters");
        return;
    }
    if (params[0] != _password)
    {
        _sendReply(client.getFd(), "464", "*", "Password incorrect");
        _disconnectClient(client.getFd());
        return;
    }
    client.setPassOk();
}

void Server::_cmdNick(Client& client, const std::vector<std::string>& params)
{
    if (params.empty())
    {
        _sendReply(client.getFd(), "431", "*", "No nickname given");
        return;
    }
    const std::string& newNick = params[0];

    // Comprobar si ya está en uso por otro cliente
    Client* existing = _getClientByNick(newNick);
    if (existing != NULL && existing != &client)
    {
        _sendReply(client.getFd(), "433", "*",
                   newNick + " :Nickname is already in use");
        return;
    }

    std::string oldPrefix = client.prefix();
    client.setNick(newNick);

    // Si ya estaba registrado, notificar el cambio a sus canales
    if (client.isRegistered())
    {
        std::string msg = ":" + oldPrefix + " NICK " + newNick + "\r\n";
        _send(client.getFd(), msg);
        for (std::map<std::string, Channel*>::iterator it = _channels.begin();
             it != _channels.end(); ++it)
        {
            if (it->second->hasMember(&client))
                it->second->broadcastExcept(msg, &client);
        }
    }
}

void Server::_cmdUser(Client& client, const std::vector<std::string>& params,
                      const std::string& trailing)
{
    if (client.isRegistered())
    {
        _sendReply(client.getFd(), "462", client.getNick(),
                   "You may not reregister");
        return;
    }
    if (params.size() < 3)
    {
        _sendReply(client.getFd(), "461", "*",
                   "USER :Not enough parameters");
        return;
    }
    client.setUser(params[0]);
    client.setRealname(trailing.empty() ? params[0] : trailing);

    if (client.isRegistered())
        _sendWelcome(client);
}

void Server::_sendWelcome(Client& client)
{
    const std::string& nick = client.getNick();
    _sendReply(client.getFd(), "001", nick,
               "Welcome to the Internet Relay Network " + client.prefix());
    _sendReply(client.getFd(), "002", nick,
               "Your host is " + _name + ", running ft_irc");
    _sendReply(client.getFd(), "003", nick,
               "This server was created just now");
    _sendReply(client.getFd(), "004", nick,
               _name + " ft_irc v1.0");
}
// ─────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────

void Server::_send(int fd, const std::string& msg)
{
    send(fd, msg.c_str(), msg.size(), 0);
}

void Server::_sendReply(int fd, const std::string& code,
                        const std::string& target, const std::string& msg)
{
    _send(fd, ":" + _name + " " + code + " " + target + " :" + msg + "\r\n");
}

Client* Server::_getClientByNick(const std::string& nick)
{
    for (std::map<int, Client*>::iterator it = _clients.begin();
         it != _clients.end(); ++it)
    {
        if (it->second->getNick() == nick)
            return it->second;
    }
    return NULL;
}

Channel* Server::_getChannel(const std::string& name)
{
    std::map<std::string, Channel*>::iterator it = _channels.find(name);
    if (it == _channels.end())
        return NULL;
    return it->second;
}

Channel* Server::_getOrCreateChannel(const std::string& name)
{
    Channel* chan = _getChannel(name);
    if (chan == NULL)
    {
        chan = new Channel(name);
        _channels[name] = chan;
    }
    return chan;
}

void Server::_cmdPart(Client& client, const std::vector<std::string>& params,
                      const std::string& trailing)
{
    if (params.empty())
    {
        _sendReply(client.getFd(), "461", client.getNick(),
                   "PART :Not enough parameters");
        return;
    }

    std::string chanName = params[0];
    Channel*    chan     = _getChannel(chanName);

    if (chan == NULL)
    {
        _sendReply(client.getFd(), "403", client.getNick(),
                   chanName + " :No such channel");
        return;
    }
    if (!chan->hasMember(&client))
    {
        _sendReply(client.getFd(), "442", client.getNick(),
                   chanName + " :You're not on that channel");
        return;
    }

    // Notificar a todos antes de salir (incluido el que sale)
    std::string reason  = trailing.empty() ? client.getNick() : trailing;
    std::string partMsg = ":" + client.prefix()
                        + " PART " + chanName
                        + " :" + reason + "\r\n";
    chan->broadcast(partMsg);

    chan->removeMember(&client);
    _cleanEmptyChannels();
}
void Server::_cmdJoin(Client& client, const std::vector<std::string>& params)
{
    if (params.empty())
    {
        _sendReply(client.getFd(), "461", client.getNick(),
                   "JOIN :Not enough parameters");
        return;
    }

    std::string chanName = params[0];
    std::string key      = params.size() > 1 ? params[1] : "";

    // Los canales empiezan por '#'
    if (chanName[0] != '#')
    {
        _sendReply(client.getFd(), "403", client.getNick(),
                   chanName + " :No such channel");
        return;
    }

    Channel* chan = _getChannel(chanName); // NO _getOrCreateChannel todavía

    if (chan != NULL) // el canal ya existe — comprobar restricciones
    {
        if (chan->hasMember(&client))
            return; // ya está dentro, ignorar silenciosamente

        if (chan->isInviteOnly() && !chan->isInvited(&client))
        {
            _sendReply(client.getFd(), "473", client.getNick(),
                       chanName + " :Cannot join channel (+i)");
            return;
        }
        if (chan->hasKey() && key != chan->getKey())
        {
            _sendReply(client.getFd(), "475", client.getNick(),
                       chanName + " :Cannot join channel (+k)");
            return;
        }
        if (chan->hasLimit() && chan->memberCount() >= chan->getLimit())
        {
            _sendReply(client.getFd(), "471", client.getNick(),
                       chanName + " :Cannot join channel (+l)");
            return;
        }
    }
    else // el canal no existe — crearlo ahora que sabemos que puede entrar
    {
        chan = new Channel(chanName);
        _channels[chanName] = chan;
    }

    // Todo ok — añadir al canal
    chan->addMember(&client);

    // El primero en entrar es operador automáticamente
    if (chan->memberCount() == 1)
        chan->addOperator(&client);

    // Notificar el JOIN a todos (incluido el nuevo)
    std::string joinMsg = ":" + client.prefix()
                        + " JOIN " + chanName + "\r\n";
    chan->broadcast(joinMsg);

    // Enviar topic si existe
    if (chan->hasTopic())
        _sendReply(client.getFd(), "332",
                   client.getNick() + " " + chanName,
                   chan->getTopic());
    else
        _sendReply(client.getFd(), "331",
                   client.getNick() + " " + chanName,
                   "No topic is set");

    // Lista de miembros
    _sendReply(client.getFd(), "353",
               client.getNick() + " = " + chanName,
               chan->namesList());
    _sendReply(client.getFd(), "366",
               client.getNick() + " " + chanName,
               "End of /NAMES list");
}

void Server::_cmdPrivmsg(Client& client, const std::vector<std::string>& params,
                         const std::string& trailing)
{
    if (params.empty())
    {
        _sendReply(client.getFd(), "411", client.getNick(),
                   "No recipient given (PRIVMSG)");
        return;
    }
    if (trailing.empty())
    {
        _sendReply(client.getFd(), "412", client.getNick(),
                   "No text to send");
        return;
    }

    const std::string& target = params[0];
    std::string msg = ":" + client.prefix()
                    + " PRIVMSG " + target
                    + " :" + trailing + "\r\n";

    if (target[0] == '#') // mensaje a canal
    {
        Channel* chan = _getChannel(target);
        if (chan == NULL)
        {
            _sendReply(client.getFd(), "403", client.getNick(),
                       target + " :No such channel");
            return;
        }
        if (!chan->hasMember(&client))
        {
            _sendReply(client.getFd(), "442", client.getNick(),
                       target + " :You're not on that channel");
            return;
        }
        chan->broadcastExcept(msg, &client);
    }
    else // mensaje privado a nick
    {
        Client* dest = _getClientByNick(target);
        if (dest == NULL)
        {
            _sendReply(client.getFd(), "401", client.getNick(),
                       target + " :No such nick");
            return;
        }
        _send(dest->getFd(), msg);
    }
}
void Server::_cmdQuit(Client& client, const std::string& trailing)
{
    std::string reason  = trailing.empty() ? "Client quit" : trailing;
    std::string quitMsg = ":" + client.prefix()
                        + " QUIT :" + reason + "\r\n";

    // Notificar a todos los canales donde estaba
    for (std::map<std::string, Channel*>::iterator it = _channels.begin();
         it != _channels.end(); ++it)
    {
        if (it->second->hasMember(&client))
        {
            it->second->broadcastExcept(quitMsg, &client);
            it->second->removeMember(&client);
        }
    }
    _cleanEmptyChannels();

    // Confirmar al cliente y desconectar
    _send(client.getFd(), "ERROR :Closing link (" + reason + ")\r\n");
    _disconnectClient(client.getFd());
}

void Server::_cmdPing(Client& client, const std::vector<std::string>& params,
                      const std::string& trailing)
{
    std::string token = trailing.empty()
                      ? (params.empty() ? _name : params[0])
                      : trailing;
    _send(client.getFd(), ":" + _name + " PONG " + _name
                        + " :" + token + "\r\n");
}
// ─────────────────────────────────────────────
// Comandos de operador
// ─────────────────────────────────────────────

void Server::_cmdKick(Client& client, const std::vector<std::string>& params,
                      const std::string& trailing)
{
    if (params.size() < 2)
    {
        _sendReply(client.getFd(), "461", client.getNick(),
                   "KICK :Not enough parameters");
        return;
    }

    const std::string& chanName  = params[0];
    const std::string& targetNick = params[1];
    std::string        reason    = trailing.empty() ? client.getNick() : trailing;

    Channel* chan = _getChannel(chanName);
    if (chan == NULL)
    {
        _sendReply(client.getFd(), "403", client.getNick(),
                   chanName + " :No such channel");
        return;
    }
    if (!chan->hasMember(&client))
    {
        _sendReply(client.getFd(), "442", client.getNick(),
                   chanName + " :You're not on that channel");
        return;
    }
    if (!chan->isOperator(&client))
    {
        _sendReply(client.getFd(), "482", client.getNick(),
                   chanName + " :You're not channel operator");
        return;
    }

    Client* target = _getClientByNick(targetNick);
    if (target == NULL || !chan->hasMember(target))
    {
        _sendReply(client.getFd(), "441", client.getNick(),
                   targetNick + " " + chanName + " :They aren't on that channel");
        return;
    }

    // Notificar a todos antes de expulsar
    std::string kickMsg = ":" + client.prefix()
                        + " KICK " + chanName
                        + " " + targetNick
                        + " :" + reason + "\r\n";
    chan->broadcast(kickMsg);

    chan->removeMember(target);
    _cleanEmptyChannels();
}
void Server::_cmdInvite(Client& client, const std::vector<std::string>& params)
{
    if (params.size() < 2)
    {
        _sendReply(client.getFd(), "461", client.getNick(),
                   "INVITE :Not enough parameters");
        return;
    }

    const std::string& targetNick = params[0];
    const std::string& chanName   = params[1];

    Channel* chan = _getChannel(chanName);
    if (chan == NULL)
    {
        _sendReply(client.getFd(), "403", client.getNick(),
                   chanName + " :No such channel");
        return;
    }
    if (!chan->hasMember(&client))
    {
        _sendReply(client.getFd(), "442", client.getNick(),
                   chanName + " :You're not on that channel");
        return;
    }
    if (!chan->isOperator(&client))
    {
        _sendReply(client.getFd(), "482", client.getNick(),
                   chanName + " :You're not channel operator");
        return;
    }

    Client* target = _getClientByNick(targetNick);
    if (target == NULL)
    {
        _sendReply(client.getFd(), "401", client.getNick(),
                   targetNick + " :No such nick");
        return;
    }
    if (chan->hasMember(target))
    {
        _sendReply(client.getFd(), "443", client.getNick(),
                   targetNick + " " + chanName + " :is already on channel");
        return;
    }

    // Registrar la invitación en el canal
    chan->addInvited(target);

    // Confirmar al operador
    _sendReply(client.getFd(), "341", client.getNick(),
               targetNick + " " + chanName);

    // Notificar al invitado
    _send(target->getFd(), ":" + client.prefix()
                         + " INVITE " + targetNick
                         + " :" + chanName + "\r\n");
}
void Server::_cmdTopic(Client& client, const std::vector<std::string>& params,
                       const std::string& trailing)
{
    if (params.empty())
    {
        _sendReply(client.getFd(), "461", client.getNick(),
                   "TOPIC :Not enough parameters");
        return;
    }

    const std::string& chanName = params[0];

    Channel* chan = _getChannel(chanName);
    if (chan == NULL)
    {
        _sendReply(client.getFd(), "403", client.getNick(),
                   chanName + " :No such channel");
        return;
    }
    if (!chan->hasMember(&client))
    {
        _sendReply(client.getFd(), "442", client.getNick(),
                   chanName + " :You're not on that channel");
        return;
    }

    // Sin trailing → consultar el topic actual
    if (trailing.empty() && params.size() == 1)
    {
        if (chan->hasTopic())
            _sendReply(client.getFd(), "332",
                       client.getNick() + " " + chanName,
                       chan->getTopic());
        else
            _sendReply(client.getFd(), "331",
                       client.getNick() + " " + chanName,
                       "No topic is set");
        return;
    }

    // Con trailing → cambiar el topic
    // Comprobar modo +t (solo operadores pueden cambiar el topic)
    if (chan->isTopicRestricted() && !chan->isOperator(&client))
    {
        _sendReply(client.getFd(), "482", client.getNick(),
                   chanName + " :You're not channel operator");
        return;
    }

    chan->setTopic(trailing);

    // Notificar el cambio a todos los miembros
    std::string topicMsg = ":" + client.prefix()
                         + " TOPIC " + chanName
                         + " :" + trailing + "\r\n";
    chan->broadcast(topicMsg);
}
void Server::_cmdMode(Client& client, const std::vector<std::string>& params)
{
    if (params.empty())
    {
        _sendReply(client.getFd(), "461", client.getNick(),
                   "MODE :Not enough parameters");
        return;
    }

    const std::string& chanName = params[0];

    Channel* chan = _getChannel(chanName);
    if (chan == NULL)
    {
        _sendReply(client.getFd(), "403", client.getNick(),
                   chanName + " :No such channel");
        return;
    }

    // Sin segundo parámetro → consultar modos actuales
    if (params.size() == 1)
    {
        _sendReply(client.getFd(), "324",
                   client.getNick() + " " + chanName,
                   chan->getModeStr());
        return;
    }

    if (!chan->isOperator(&client))
    {
        _sendReply(client.getFd(), "482", client.getNick(),
                   chanName + " :You're not channel operator");
        return;
    }

    const std::string& modeStr = params[1];
    size_t             argIdx  = 2; // índice del siguiente argumento
    bool               adding  = true;
    std::string        appliedModes;
    std::string        appliedArgs;

    for (size_t i = 0; i < modeStr.size(); i++)
    {
        char m = modeStr[i];

        if (m == '+') { adding = true;  continue; }
        if (m == '-') { adding = false; continue; }

        if (m == 'i')
        {
            chan->setInviteOnly(adding);
            appliedModes += m;
        }
        else if (m == 't')
        {
            chan->setTopicRestricted(adding);
            appliedModes += m;
        }
        else if (m == 'k')
        {
            if (adding)
            {
                if (argIdx >= params.size())
                {
                    _sendReply(client.getFd(), "461", client.getNick(),
                               "MODE :Not enough parameters");
                    return;
                }
                chan->setKey(params[argIdx]);
                appliedArgs += " " + params[argIdx];
                argIdx++;
            }
            else
                chan->removeKey();
            appliedModes += m;
        }
        else if (m == 'o')
        {
            if (argIdx >= params.size())
            {
                _sendReply(client.getFd(), "461", client.getNick(),
                           "MODE :Not enough parameters");
                return;
            }
            Client* target = _getClientByNick(params[argIdx]);
            if (target != NULL && chan->hasMember(target))
            {
                if (adding)
                    chan->addOperator(target);
                else
                    chan->removeOperator(target);
                appliedModes += m;
                appliedArgs  += " " + params[argIdx];
            }
            argIdx++;
        }
        else if (m == 'l')
        {
            if (adding)
            {
                if (argIdx >= params.size())
                {
                    _sendReply(client.getFd(), "461", client.getNick(),
                               "MODE :Not enough parameters");
                    return;
                }
                int limit = std::atoi(params[argIdx].c_str());
                if (limit > 0)
                {
                    chan->setLimit(limit);
                    appliedArgs += " " + params[argIdx];
                }
                argIdx++;
            }
            else
                chan->removeLimit();
            appliedModes += m;
        }
    }

    // Notificar el cambio solo si se aplicó algo
    if (!appliedModes.empty())
    {
        std::string modeMsg = ":" + client.prefix()
                            + " MODE " + chanName
                            + " " + (adding ? "+" : "-") + appliedModes
                            + appliedArgs + "\r\n";
        chan->broadcast(modeMsg);
    }
}