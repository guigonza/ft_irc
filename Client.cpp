/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Guille <Guille@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/23 11:20:28 by Guille            #+#    #+#             */
/*   Updated: 2026/03/23 11:36:29 by Guille           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include <unistd.h>

// ─────────────────────────────────────────────
// OCF
// ─────────────────────────────────────────────

Client::Client(int fd, const std::string& hostname)
    : _fd(fd),
      _nick("*"),
      _user(""),
      _realname(""),
	  _hostname(hostname),
      _passOk(false),
      _nickSet(false),
      _userSet(false),
      _inbuf("")
{}

Client::~Client()
{
    // El fd lo cierra Server — Client no es responsable de cerrarlo
}

// ─────────────────────────────────────────────
// Buffer de entrada
// ─────────────────────────────────────────────

void Client::appendBuffer(const char* data, int len)
{
    _inbuf.append(data, len);
}

bool Client::getLine(std::string& line)
{
    // Buscar \r\n — terminador estándar IRC
    size_t pos = _inbuf.find("\r\n");

    if (pos == std::string::npos)
    {
        // Algunos clientes mandan solo \n — también lo aceptamos
        pos = _inbuf.find("\n");
        if (pos == std::string::npos)
            return false; // línea incompleta, esperar más datos
    }

    line = _inbuf.substr(0, pos);
    _inbuf.erase(0, pos + (_inbuf[pos] == '\r' ? 2 : 1));
    return true;
}
// ─────────────────────────────────────────────
// Estado de registro
// ─────────────────────────────────────────────

bool Client::isRegistered() const
{
    return _passOk && _nickSet && _userSet;
}

void Client::setPassOk()
{
    _passOk = true;
}

void Client::setNick(const std::string& nick)
{
    _nick    = nick;
    _nickSet = true;
}

void Client::setUser(const std::string& user)
{
    _user    = user;
    _userSet = true;
}

void Client::setRealname(const std::string& realname)
{
    _realname = realname;
}
const std::string& Client::getHostname() const
{
    return _hostname;
}
// ─────────────────────────────────────────────
// Getters
// ─────────────────────────────────────────────

int Client::getFd() const
{
    return _fd;
}

const std::string& Client::getNick() const
{
    return _nick;
}

const std::string& Client::getUser() const
{
    return _user;
}

const std::string& Client::getRealname() const
{
    return _realname;
}

std::string Client::prefix() const
{
    return _nick + "!" + _user + "@" + _hostname;
}