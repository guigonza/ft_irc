/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Guille <Guille@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/23 11:43:28 by Guille            #+#    #+#             */
/*   Updated: 2026/03/23 11:43:31 by Guille           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <string>
# include <set>

class Client;

class Channel
{
	public:

		Channel(const std::string& name);
		~Channel();

		// Miembros
		void  addMember     (Client* client);
		void  removeMember  (Client* client);
		bool  hasMember     (Client* client) const;
		int   memberCount   () const;

		// Operadores
		void  addOperator   (Client* client);
		void  removeOperator(Client* client);
		bool  isOperator    (Client* client) const;

		// Invitaciones
		void  addInvited    (Client* client);
		bool  isInvited     (Client* client) const;

		// Mensajes
		void  broadcast          (const std::string& msg);
		void  broadcastExcept    (const std::string& msg, Client* exclude);

		// Topic
		void               setTopic (const std::string& topic);
		const std::string& getTopic () const;
		bool               hasTopic () const;

		// Modos
		void  setInviteOnly      (bool on);
		void  setTopicRestricted (bool on);
		void  setKey             (const std::string& key);
		void  removeKey          ();
		void  setLimit           (int limit);
		void  removeLimit        ();

		bool               isInviteOnly      () const;
		bool               isTopicRestricted () const;
		bool               hasKey            () const;
		const std::string& getKey            () const;
		bool               hasLimit          () const;
		int                getLimit          () const;

		// Info
		const std::string& getName    () const;
		std::string        namesList  () const;
		std::string        getModeStr () const;

	private:

		// OCF — prohibido copiar
		Channel();
		Channel(const Channel&);
		Channel& operator=(const Channel&);

		std::string       _name;
		std::string       _topic;
		std::string       _key;
		int               _limit;
		bool              _inviteOnly;
		bool              _topicRestricted;
		bool              _hasKey;
		bool              _hasLimit;

		std::set<Client*> _members;
		std::set<Client*> _operators;
		std::set<Client*> _invited;
};

#endif