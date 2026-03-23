/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Guille <Guille@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/23 11:43:49 by Guille            #+#    #+#             */
/*   Updated: 2026/03/23 11:50:37 by Guille           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include "Client.hpp"
#include <sys/socket.h>
#include <sstream>

// ─────────────────────────────────────────────
// OCF
// ─────────────────────────────────────────────

Channel::Channel(const std::string& name)
    : _name(name),
      _topic(""),
      _key(""),
      _limit(0),
      _inviteOnly(false),
      _topicRestricted(false),
      _hasKey(false),
      _hasLimit(false)
{}

Channel::~Channel()
{
    // Channel no posee a los clientes — no hace delete
    // Solo limpia los sets de punteros
    _members.clear();
    _operators.clear();
    _invited.clear();
}
// ─────────────────────────────────────────────
// Miembros
// ─────────────────────────────────────────────

void Channel::addMember(Client* client)
{
    _members.insert(client);
}

void Channel::removeMember(Client* client)
{
    _members.erase(client);
    _operators.erase(client); // si era op, deja de serlo al salir
    _invited.erase(client);   // limpiar invitación si la tenía
}

bool Channel::hasMember(Client* client) const
{
    return _members.count(client) > 0;
}

int Channel::memberCount() const
{
    return static_cast<int>(_members.size());
}

// ─────────────────────────────────────────────
// Operadores
// ─────────────────────────────────────────────

void Channel::addOperator(Client* client)
{
    if (hasMember(client))
        _operators.insert(client);
}

void Channel::removeOperator(Client* client)
{
    _operators.erase(client);
}

bool Channel::isOperator(Client* client) const
{
    return _operators.count(client) > 0;
}
// ─────────────────────────────────────────────
// Invitaciones
// ─────────────────────────────────────────────

void Channel::addInvited(Client* client)
{
    _invited.insert(client);
}

bool Channel::isInvited(Client* client) const
{
    return _invited.count(client) > 0;
}
// ─────────────────────────────────────────────
// Mensajes
// ─────────────────────────────────────────────

void Channel::broadcast(const std::string& msg)
{
    for (std::set<Client*>::iterator it = _members.begin();
         it != _members.end(); ++it)
    {
        send((*it)->getFd(), msg.c_str(), msg.size(), 0);
    }
}

void Channel::broadcastExcept(const std::string& msg, Client* exclude)
{
    for (std::set<Client*>::iterator it = _members.begin();
         it != _members.end(); ++it)
    {
        if (*it != exclude)
            send((*it)->getFd(), msg.c_str(), msg.size(), 0);
    }
}
// ─────────────────────────────────────────────
// Topic
// ─────────────────────────────────────────────

void Channel::setTopic(const std::string& topic)
{
    _topic = topic;
}

const std::string& Channel::getTopic() const
{
    return _topic;
}

bool Channel::hasTopic() const
{
    return !_topic.empty();
}
// ─────────────────────────────────────────────
// Modos
// ─────────────────────────────────────────────

void Channel::setInviteOnly(bool on)
{
    _inviteOnly = on;
}

void Channel::setTopicRestricted(bool on)
{
    _topicRestricted = on;
}

void Channel::setKey(const std::string& key)
{
    _key    = key;
    _hasKey = true;
}

void Channel::removeKey()
{
    _key    = "";
    _hasKey = false;
}

void Channel::setLimit(int limit)
{
    _limit    = limit;
    _hasLimit = true;
}

void Channel::removeLimit()
{
    _limit    = 0;
    _hasLimit = false;
}

bool Channel::isInviteOnly() const      { return _inviteOnly; }
bool Channel::isTopicRestricted() const { return _topicRestricted; }
bool Channel::hasKey() const            { return _hasKey; }
bool Channel::hasLimit() const          { return _hasLimit; }

const std::string& Channel::getKey() const   { return _key; }
int                Channel::getLimit() const { return _limit; }

// ─────────────────────────────────────────────
// Info
// ─────────────────────────────────────────────

const std::string& Channel::getName() const
{
    return _name;
}

std::string Channel::namesList() const
{
    std::string list;
    for (std::set<Client*>::const_iterator it = _members.begin();
         it != _members.end(); ++it)
    {
        if (!list.empty())
            list += " ";
        if (isOperator(*it))
            list += "@"; // prefijo @ para operadores
        list += (*it)->getNick();
    }
    return list;
}

std::string Channel::getModeStr() const
{
    std::string modes = "+";
    std::string args;

    if (_inviteOnly)      modes += "i";
    if (_topicRestricted) modes += "t";
    if (_hasKey)        { modes += "k"; args += " " + _key; }
    if (_hasLimit)
    {
        std::ostringstream oss;
        oss << _limit;
        modes += "l";
        args  += " " + oss.str();
    }

    if (modes == "+")
        return "";
    return modes + args;
}
