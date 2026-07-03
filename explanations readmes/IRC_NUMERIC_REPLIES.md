# IRC Numeric Replies — ft\_irc

Lista completa de códigos numéricos que el servidor `ft_irc` puede enviar, ordenados por categoría.

---

## Registro y bienvenida

| Código | Nombre | Cuándo se envía | Ejemplo |
|---|---|---|---|
| **001** | RPL_WELCOME | Registro exitoso | `:ft_irc 001 Bob :Welcome to the Internet Relay Network Bob!b@127.0.0.1` |
| **002** | RPL_YOURHOST | Info del servidor | `:ft_irc 002 Bob :Your host is ft_irc, running ft_irc` |
| **003** | RPL_CREATED | Fecha de creación | `:ft_irc 003 Bob :This server was created just now` |
| **004** | RPL_MYINFO | Versión del servidor | `:ft_irc 004 Bob :ft_irc ft_irc v1.0` |

---

## Canales — información

| Código | Nombre | Cuándo se envía | Ejemplo |
|---|---|---|---|
| **331** | RPL_NOTOPIC | El canal no tiene topic | `:ft_irc 331 Bob #general :No topic is set` |
| **332** | RPL_TOPIC | Topic actual del canal | `:ft_irc 332 Bob #general :Bienvenidos al canal` |
| **324** | RPL_CHANNELMODEIS | Modos actuales del canal | `:ft_irc 324 Bob #general +nt` |
| **353** | RPL_NAMREPLY | Lista de miembros del canal | `:ft_irc 353 Bob = #general :@Alice Bob` |
| **366** | RPL_ENDOFNAMES | Fin de lista de miembros | `:ft_irc 366 Bob #general :End of /NAMES list` |
| **341** | RPL_INVITING | Confirmación de invitación | `:ft_irc 341 Bob Alice #general` |

---

## Errores — nickname / usuario

| Código | Nombre | Cuándo se envía | Ejemplo |
|---|---|---|---|
| **431** | ERR_NONICKNAMEGIVEN | No se proporcionó nick | `:ft_irc 431 * :No nickname given` |
| **433** | ERR_NICKNAMEINUSE | El nick ya está en uso | `:ft_irc 433 * Bob :Nickname is already in use` |
| **401** | ERR_NOSUCHNICK | El destinatario no existe | `:ft_irc 401 Alice Bob :No such nick` |
| **451** | ERR_NOTREGISTERED | Comando enviado sin registrar | `:ft_irc 451 * :You have not registered` |
| **462** | ERR_ALREADYREGISTRED | Intento de re-registro | `:ft_irc 462 Alice :You may not reregister` |
| **464** | ERR_PASSWDMISMATCH | Contraseña incorrecta | `:ft_irc 464 * :Password incorrect` |

---

## Errores — canales

| Código | Nombre | Cuándo se envía | Ejemplo |
|---|---|---|---|
| **403** | ERR_NOSUCHCHANNEL | El canal no existe | `:ft_irc 403 Alice #nonexist :No such channel` |
| **442** | ERR_NOTONCHANNEL | No estás en el canal | `:ft_irc 442 Alice #general :You're not on that channel` |
| **441** | ERR_USERNOTINCHANNEL | El usuario no está en el canal | `:ft_irc 441 Alice Bob #general :They aren't on that channel` |
| **443** | ERR_USERONCHANNEL | El usuario ya está en el canal | `:ft_irc 443 Alice Bob #general :is already on channel` |
| **471** | ERR_CHANNELISFULL | Canal lleno (+l) | `:ft_irc 471 Alice #general :Cannot join channel (+l)` |
| **473** | ERR_INVITEONLYCHAN | Canal solo invitación (+i) | `:ft_irc 473 Alice #general :Cannot join channel (+i)` |
| **475** | ERR_BADCHANNELKEY | Clave incorrecta (+k) | `:ft_irc 475 Alice #general :Cannot join channel (+k)` |
| **482** | ERR_CHANOPRIVSNEEDED | No eres operador del canal | `:ft_irc 482 Alice #general :You're not channel operator` |

---

## Errores — comandos / parámetros

| Código | Nombre | Cuándo se envía | Ejemplo |
|---|---|---|---|
| **411** | ERR_NORECIPIENT | PRIVMSG sin destinatario | `:ft_irc 411 Alice :No recipient given (PRIVMSG)` |
| **412** | ERR_NOTEXTTOSEND | PRIVMSG sin texto | `:ft_irc 412 Alice :No text to send` |
| **421** | ERR_UNKNOWNCOMMAND | Comando desconocido | `:ft_irc 421 Alice XYZ :Unknown command` |
| **461** | ERR_NEEDMOREPARAMS | Faltan parámetros | `:ft_irc 461 Alice JOIN :Not enough parameters` |

---

## Códigos ignorados por el servidor

El servidor **no responde** a:

| Comando | Motivo |
|---|---|
| `CAP` | Capability negotiation — se ignora silenciosamente |
| `WHO` | Algunos clientes lo envían automáticamente — se ignora |

---

## Notas

- El formato siempre es: `:<server_name> <código> <target> :<mensaje>`
- `<target>` puede ser un nick, `*` si el cliente aún no está registrado, o `nick #canal` según el contexto
- Todos los mensajes terminan con `\r\n` (CRLF)
