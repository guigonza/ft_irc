*This project has been created as part of the 42 curriculum by guigonza, alcarril, carbon-m.*

# ft_irc

## Description

ft_irc is a C++98 IRC server that accepts TCP/IP client connections, authenticates users, and lets them join channels, exchange private messages, and manage channels through standard IRC commands.

The mandatory implementation focuses on:

- non-blocking socket I/O
- a single polling loop to handle all clients
- IRC registration with PASS, NICK, and USER
- channel operations such as JOIN, PART, PRIVMSG, QUIT, PING, KICK, INVITE, TOPIC, and MODE
- support for channel operators and regular users

## Instructions

### Requirements

- C++98 compiler
- `make`

### Build

```bash
make
```

This produces the mandatory executable: `ircserv`.

### Optional bonus build

```bash
make bonus
```

This produces the bonus executable: `ircserv_bonus`.

### Clean

```bash
make clean
make fclean
make re
```

### Run

```bash
./ircserv <port> <password>
```

Example:

```bash
./ircserv 6667 mypassword
```

Connect with an IRC client using the same port and password.

## Resources

### References

- RFC 1459 - Internet Relay Chat Protocol
- RFC 2812 - Internet Relay Chat: Client Protocol
- IRC command documentation from common IRC client manuals
- `man` pages for `socket`, `bind`, `listen`, `accept`, `recv`, `send`, `poll`, `fcntl`, and related networking calls

### AI usage

AI was used only as a support tool to:

- search for relevant IRC protocol information and command references
- check for implementation mistakes and subject mismatches
- review example implementations and compare design options

All generated suggestions were reviewed manually before being applied.

## Notes

- The project is designed around a single event loop with non-blocking file descriptors.
- The bonus code path is separate from the mandatory server build.