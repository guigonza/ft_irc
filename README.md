*This project has been created as part of the 42 curriculum by guigonza, carbon-m, alcarril.*

## Description

**ft\_irc** is an IRC (Internet Relay Chat) server written in C++98, compatible with standard IRC clients such as HexChat, irssi, and mIRC. The project implements the core IRC protocol as defined in RFC 1459, including channel management, operator commands, private messaging, and a DCC file transfer handshake relay.

The server is single-threaded and uses `poll()` for multiplexing I/O across multiple clients. It also includes an integrated bot that responds to commands prefixed with `!` in channels.

### Features

- **Authentication:** PASS, NICK, USER flow with password verification
- **Channels:** JOIN, PART, TOPIC, channel modes (invite-only `+i`, topic restriction `+t`, key `+k`, user limit `+l`, operator `+o`)
- **Messaging:** PRIVMSG to channels and private messages to users
- **Operator commands:** KICK, INVITE, MODE
- **DCC file transfer:** Detects and logs DCC SEND handshakes (CTCP `\x01DCC SEND\x01`) and relays them to the target without interfering — the actual file transfer happens P2P between clients
- **Integrated bot:** Responds to `!help`, `!info`, `!list`, `!ping`, `!echo`, `!welcome` in channels

## Instructions

### Requirements

- C++ compiler with C++98 support (g++, clang++)
- GNU Make
- `poll()` support (POSIX)

### Compilation

```bash
make        # builds the ircserv binary
make bonus  # builds the bonus version (ircserv_bonus)
make clean  # removes object files
make fclean # removes object files and binaries
make re     # full recompilation
```

Compilation flags: `-Wall -Wextra -Werror -std=c++98`

### Execution

```bash
./ircserv <port> <password>
```

- `<port>`: listening port (e.g., 6667)
- `<password>`: server password required for client connection

Example:

```bash
./ircserv 6667 mypassword
```

### Connecting with a client

**HexChat:**
1. Add network → server `localhost/6667`, password `mypassword`
2. Connect and use standard IRC commands: `/join #channel`, `/msg user hello`, `/dcc send user file.txt`

**Netcat (raw protocol):**
```bash
nc localhost 6667
PASS mypassword
NICK mynick
USER myuser 0 * :My Real Name
JOIN #channel
PRIVMSG #channel :Hello everyone
```

### Bonus — Bot

The integrated bot joins every channel created on the server. Commands are triggered by messages starting with `!`:

```
!help     — Show available commands
!info     — Bot information
!ping     — Ping the bot
!echo     — Echo a message
!welcome  — Welcome message
```

### DCC File Transfer

The server does **not** handle file data — it only relays the DCC SEND handshake. Example DCC offer from Alice to Bob:

```
PRIVMSG Bob :\x01DCC SEND f.txt 2130706433 5001 15\x01
```

The server detects the `\x01DCC SEND` prefix, logs the transfer, and forwards the message to Bob unchanged. Bob's client then connects directly to Alice for the actual file transfer.

## Project structure

| File | Purpose |
|---|---|
| `Server.cpp` / `Server.hpp` | Main server: socket setup, event loop, command dispatch |
| `Client.cpp` / `Client.hpp` | Client state: buffer, authentication, nick/user |
| `Channel.cpp` / `Channel.hpp` | Channel management: members, operators, modes, topic |
| `FileHandler.cpp` / `FileHandler.hpp` | DCC message parsing utilities |
| `Bot.cpp` / `Bot.hpp` | Integrated IRC bot with command handling |
| `main.cpp` | Entry point, port/password parsing |
| `IRC_CHEAT_SHEET.md` | Quick reference for IRC commands and protocol |
| `DCC_CHEATSHEET.md` | DCC file transfer documentation and testing guide |

## Resources

### IRC protocol references

- [RFC 1459 — Internet Relay Chat Protocol](https://datatracker.ietf.org/doc/html/rfc1459)
- [IRCv3 specifications](https://ircv3.net/)
- [Modern IRC Docs](https://modern.ircdocs.horse/)
- [DCC specification (irchelp)](https://www.irchelp.org/protocol/dccspec.html)
- [CTCP specification (IETF draft)](https://datatracker.ietf.org/doc/html/draft-oakley-irc-ctcp-02)

### How AI was used

AI (opencode / Claude) was used for the following tasks:

- **Code assistance:** Generating the initial structure of DCC parsing functions (`isDCCMessage`, `parseDCCSend`) and the DCC detection block in `_processLine`
- **Debugging:** Identifying issues with the original in-memory file handler and guiding the transition to the DCC relay model
- **Documentation:** Writing the DCC cheat sheet and this README
- **Code review:** Ensuring compliance with C++98, the禁止 copy (OCF) rules, and the project-specific function restrictions

All AI-generated code was reviewed, tested, and validated by the team. The core server logic (event loop, channel management, command dispatch, authentication) was written entirely by the team.
