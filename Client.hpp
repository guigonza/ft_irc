/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: Guille <Guille@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/23 11:13:58 by Guille            #+#    #+#             */
/*   Updated: 2026/03/23 11:26:39 by Guille           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>

class Client
{
	public:

		Client(int fd, const std::string& hostname);
		~Client();

		// Buffer de entrada
		void        appendBuffer(const char* data, int len);
		bool        getLine(std::string& line);

		// Estado de registro
		bool        isRegistered() const;
		void        setPassOk();
		void        setNick(const std::string& nick);
		void        setUser(const std::string& user);
		void        setRealname(const std::string& realname);

		// Getters
		int                getFd()       const;
		const std::string& getNick()     const;
		const std::string& getUser()     const;
		const std::string& getRealname() const;
		const std::string& getHostname() const;
		std::string        prefix()      const; // nick!user@host

	private:

		// OCF — prohibido copiar
		Client();
		Client(const Client&);
		Client& operator=(const Client&);

		int         _fd;
		std::string _nick;
		std::string _user;
		std::string _realname;
		std::string _hostname;
		bool        _passOk;
		bool        _nickSet;
		bool        _userSet;
		std::string _inbuf;
};

#endif