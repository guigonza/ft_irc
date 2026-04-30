/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Guille <Guille@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 12:00:00 by Guille            #+#    #+#             */
/*   Updated: 2026/04/28 12:00:00 by Guille           ###   ########.fr        */
/*                                                                            */
/* ************************************************************************** */

#include "Bot.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <cerrno>

// ─────────────────────────────────────────────
// OCF
// ─────────────────────────────────────────────

Bot::Bot(const std::string& host, int port, const std::string& password)
    : _host(host),
      _port(port),
      _password(password),
      _nick("ft_bot"),
      _fd(-1),
      _inbuf(""),
      _commandCount(0),
      _startTime(std::time(NULL)),
      _authenticated(false)
{
    _connect();
    _authenticate();
    
    // Unirse a canales por defecto
    _send("JOIN #general\r\n");
    _send("JOIN #random\r\n");
    
    std::cout << "[Bot] Bot iniciado y conectado. Esperando comandos..." << std::endl;
}

Bot::~Bot()
{
    if (_fd != -1)
    {
        _send("QUIT :Bot shutting down\r\n");
        close(_fd);
    }
}

// ─────────────────────────────────────────────
// Conexión
// ─────────────────────────────────────────────

void Bot::_connect()
{
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd == -1)
        throw std::runtime_error("socket() failed");

    fcntl(_fd, F_SETFL, O_NONBLOCK);

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(_port);
    
    // Convertir "localhost" a "127.0.0.1"
    std::string ipAddr = _host;
    if (_host == "localhost")
        ipAddr = "127.0.0.1";
    
    if (inet_pton(AF_INET, ipAddr.c_str(), &addr.sin_addr) <= 0)
        throw std::runtime_error("inet_pton() failed");

    int ret = connect(_fd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret == -1 && errno != EINPROGRESS)
        throw std::runtime_error("connect() failed");

    // Esperar a que la conexión se establezca
    int timeout = 100; // 10 segundos máximo
    while (timeout-- > 0)
    {
        fd_set writeSet;
        FD_ZERO(&writeSet);
        FD_SET(_fd, &writeSet);
        
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000; // 100ms
        
        int sr = select(_fd + 1, NULL, &writeSet, NULL, &tv);
        if (sr > 0)
        {
            int so_error = 0;
            socklen_t so_error_len = sizeof(so_error);
            getsockopt(_fd, SOL_SOCKET, SO_ERROR, &so_error, &so_error_len);
            if (so_error == 0)
            {
                std::cout << "[Bot] Conectado a " << _host << ":" << _port << std::endl;
                return;
            }
        }
    }
    throw std::runtime_error("connect() timeout");
}

void Bot::_authenticate()
{
    _send("PASS " + _password + "\r\n");
    _send("NICK " + _nick + "\r\n");
    _send("USER bot 0 * :IRC Bot\r\n");

    // Esperar a mensajes de bienvenida o timeout
    int timeout = 50; // 5 segundos máximo
    while (timeout-- > 0 && !_authenticated)
    {
        std::string line = _receive();
        if (!line.empty())
        {
            // Buscar código 001 (welcome)
            if (line.find(" 001 ") != std::string::npos)
            {
                _authenticated = true;
                std::cout << "[Bot] Autenticado correctamente" << std::endl;
                break;
            }
        }
        usleep(100000); // 100ms
    }
    
    if (!_authenticated)
        std::cout << "[Bot] Advertencia: Autenticación podría no estar completa" << std::endl;
}

void Bot::_disconnect()
{
    if (_fd != -1)
    {
        close(_fd);
        _fd = -1;
    }
}

// ─────────────────────────────────────────────
// Recepción y envío
// ─────────────────────────────────────────────

std::string Bot::_receive()
{
    // Primero, intentar extraer una línea del buffer existente
    size_t pos = _inbuf.find("\r\n");
    if (pos == std::string::npos)
        pos = _inbuf.find("\n");
    
    if (pos != std::string::npos)
    {
        std::string line = _inbuf.substr(0, pos);
        _inbuf.erase(0, pos + (_inbuf[pos] == '\r' ? 2 : 1));
        return line;
    }

    // Si no hay línea completa en el buffer, recibir datos nuevos
    char buf[512];
    int bytes = recv(_fd, buf, sizeof(buf) - 1, 0);

    if (bytes == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return ""; // No hay datos todavía
        return "";
    }
    if (bytes == 0)
    {
        std::cout << "[Bot] Conexión cerrada por el servidor" << std::endl;
        _disconnect();
        return "";
    }

    buf[bytes] = '\0';
    _inbuf.append(buf, bytes);

    // Ahora intentar extraer una línea del buffer completo
    pos = _inbuf.find("\r\n");
    if (pos == std::string::npos)
        pos = _inbuf.find("\n");
    
    if (pos != std::string::npos)
    {
        std::string line = _inbuf.substr(0, pos);
        _inbuf.erase(0, pos + (_inbuf[pos] == '\r' ? 2 : 1));
        return line;
    }
    
    return "";
}

void Bot::_send(const std::string& msg)
{
    int ret = send(_fd, msg.c_str(), msg.size(), 0);
    if (ret < 0)
    {
        std::cerr << "[Bot] Error sending: " << strerror(errno) << std::endl;
    }
}

// ─────────────────────────────────────────────
// Bucle principal
// ─────────────────────────────────────────────

void Bot::run()
{
    while (true)
    {
        // Procesar múltiples líneas sin bloquear
        std::string line;
        int count = 0;
        while ((line = _receive()).empty() == false && count < 10)
        {
            _handleMessage(line);
            _updateChannels(line);
            count++;
        }
        
        usleep(50000); // 50ms para no saturar CPU
    }
}

// ─────────────────────────────────────────────
// Manejo de mensajes
// ─────────────────────────────────────────────

void Bot::_handleMessage(const std::string& line)
{
    // Formato: :nick!user@host PRIVMSG #canal :!comando args
    
    if (line.empty())
        return;

    std::string prefix;
    std::string command;
    std::string channel;
    std::string trailing;

    size_t pos = 0;

    // ¿Tiene prefix?
    if (line[0] == ':')
    {
        size_t spacePos = line.find(' ');
        if (spacePos != std::string::npos)
        {
            prefix = line.substr(1, spacePos - 1);
            pos = spacePos + 1;
        }
        else
            return;
    }

    // Comando
    size_t cmdStart = pos;
    size_t cmdEnd = line.find(' ', pos);
    if (cmdEnd == std::string::npos)
        return;
    command = line.substr(cmdStart, cmdEnd - cmdStart);
    pos = cmdEnd + 1;

    // Parámetro (canal o nick)
    size_t chanStart = pos;
    size_t chanEnd = line.find(' ', pos);
    if (chanEnd == std::string::npos)
        return;
    channel = line.substr(chanStart, chanEnd - chanStart);
    pos = chanEnd + 1;

    // Trailing (mensaje) - buscar ':'
    size_t colonPos = line.find(':', pos);
    if (colonPos != std::string::npos)
        trailing = line.substr(colonPos + 1);

    // Normalizar comando a mayúsculas
    for (size_t i = 0; i < command.size(); i++)
        command[i] = std::toupper(command[i]);

    // Solo procesar PRIVMSG con "!"
    if (command == "PRIVMSG" && !trailing.empty() && trailing[0] == '!')
    {
        // Extraer nick del prefix (nick!user@host)
        size_t nickEnd = prefix.find('!');
        std::string nick = (nickEnd != std::string::npos) ? prefix.substr(0, nickEnd) : prefix;

        std::cout << "[Bot] Comando: " << trailing << " de " << nick 
                  << " en " << channel << std::endl;

        _processCommand(channel, nick, trailing);
    }
}

void Bot::_updateChannels(const std::string& line)
{
    // Rastrear JOIN/PART para mantener lista de canales
    std::string command;
    std::istringstream ss(line);

    if (line[0] == ':')
    {
        std::string prefix;
        ss >> prefix;
    }

    ss >> command;

    for (size_t i = 0; i < command.size(); i++)
        command[i] = std::toupper(command[i]);

    if (command == "JOIN")
    {
        std::string channel;
        ss >> channel;
        
        // Comprobar si ya existe
        for (size_t i = 0; i < _joinedChannels.size(); i++)
        {
            if (_joinedChannels[i] == channel)
                return;
        }
        _joinedChannels.push_back(channel);
    }
    else if (command == "PART")
    {
        std::string channel;
        ss >> channel;
        
        for (size_t i = 0; i < _joinedChannels.size(); i++)
        {
            if (_joinedChannels[i] == channel)
            {
                _joinedChannels.erase(_joinedChannels.begin() + i);
                break;
            }
        }
    }
}

// ─────────────────────────────────────────────
// Procesamiento de comandos
// ─────────────────────────────────────────────

void Bot::_processCommand(const std::string& channel,
                          const std::string& user,
                          const std::string& command)
{
    std::string cmd = command.substr(1); // Quitar el "!"

    std::cout << "[Bot] Comando recibido de " << user << " en " 
              << channel << ": " << cmd << std::endl;

    if (cmd.find("help") == 0)
        _cmdHelp(channel);
    else if (cmd.find("echo ") == 0)
        _cmdEcho(channel, cmd.substr(5));
    else if (cmd.find("info") == 0)
        _cmdInfo(channel);
    else if (cmd.find("contador") == 0)
        _cmdContador(channel, cmd.substr(8));
    else if (cmd.find("hora") == 0)
        _cmdHora(channel);
    else if (cmd.find("usuarios") == 0)
        _cmdUsuarios(channel);
    else if (cmd.find("ping") == 0)
        _cmdPing(channel);
    else
    {
        std::string msg = "PRIVMSG " + channel + 
            " :Comando desconocido. Usa !help para ver comandos disponibles\r\n";
        _send(msg);
    }
}

// ─────────────────────────────────────────────
// Comandos
// ─────────────────────────────────────────────

void Bot::_cmdHelp(const std::string& channel)
{
    std::string msg = "PRIVMSG " + channel + 
        " :Comandos: !help !echo <msg> !info !contador [reset] !hora !usuarios !ping\r\n";
    _send(msg);
}

void Bot::_cmdEcho(const std::string& channel, const std::string& args)
{
    if (args.empty())
    {
        std::string msg = "PRIVMSG " + channel + " :Uso: !echo <mensaje>\r\n";
        _send(msg);
        return;
    }
    
    std::string msg = "PRIVMSG " + channel + " :" + args + "\r\n";
    _send(msg);
}

void Bot::_cmdInfo(const std::string& channel)
{
    time_t now = std::time(NULL);
    int uptime = static_cast<int>(now - _startTime);
    
    std::ostringstream oss;
    oss << uptime;
    
    std::string msg = "PRIVMSG " + channel + 
        " :Servidor ft_irc v1.0 | Uptime: " + oss.str() + "s\r\n";
    _send(msg);
}

void Bot::_cmdContador(const std::string& channel, const std::string& args)
{
    std::string trimmedArgs = args;
    
    // Trim leading spaces
    size_t start = trimmedArgs.find_first_not_of(" \t\r\n");
    if (start != std::string::npos)
        trimmedArgs = trimmedArgs.substr(start);
    
    if (trimmedArgs.find("reset") != std::string::npos)
    {
        _commandCount = 0;
        std::string msg = "PRIVMSG " + channel + " :Contador reiniciado a 0\r\n";
        _send(msg);
    }
    else
    {
        _commandCount++;
        
        std::ostringstream oss;
        oss << _commandCount;
        
        std::string msg = "PRIVMSG " + channel + 
            " :Contador: " + oss.str() + "\r\n";
        _send(msg);
    }
}

void Bot::_cmdHora(const std::string& channel)
{
    time_t now = std::time(NULL);
    struct tm* timeinfo = std::localtime(&now);
    char buffer[50];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

    std::string msg = "PRIVMSG " + channel + " :" + buffer + "\r\n";
    _send(msg);
}

void Bot::_cmdUsuarios(const std::string& channel)
{
    std::string msg = "PRIVMSG " + channel + 
        " :Comando USUARIOS: Solicita NAMES al servidor. Respuesta en el siguiente mensaje.\r\n";
    _send(msg);
    
    // Enviar comando NAMES
    _send("NAMES " + channel + "\r\n");
}

void Bot::_cmdPing(const std::string& channel)
{
    std::string msg = "PRIVMSG " + channel + " :Pong!\r\n";
    _send(msg);
}
