# IRC Cheat Sheet (Servidor ft_irc)

Guia rapida para probar clientes contra este servidor.

## 1. Como conectarse al servidor

Con netcat (ejemplo):

```bash
nc 127.0.0.1 6667
```

Importante:
- IRC usa lineas terminadas en `\r\n` (CRLF).
- Si escribes a mano en terminal, `Enter` suele enviar `\n`; muchos clientes IRC ya manejan esto bien.

## 2. Estructura basica de un mensaje IRC

Formato general:

```text
[:prefix] COMANDO [param1 param2 ...] [:trailing]
```

- `prefix` es opcional.
- `COMANDO` puede venir en minusculas o mayusculas (el servidor normaliza).
- `param1 param2 ...` son parametros separados por espacios.
- `:trailing` es el ultimo campo y puede contener espacios.

### Que poner realmente en el chat

Si estas usando `nc` o `telnet`, escribes lineas como estas (una por Enter):

```text
PASS 1234
NICK pepe
USER pepe 0 * :Pepe Lopez
JOIN #general
PRIVMSG #general :hola equipo
PRIVMSG ana :hola ana, te escribo por privado
PART #general :me salgo un momento
QUIT :hasta luego
```

Si usas un cliente IRC grafico (HexChat, irssi, etc.), normalmente escribes comandos con `/` y el cliente los convierte al protocolo IRC:

```text
/nick pepe
/join #general
/msg ana hola ana
```

El texto normal que escribes en la caja de chat, el cliente lo manda como `PRIVMSG`.

### Prefix: que es y cuando aparece

El `prefix` casi siempre lo veras en mensajes que RECIBES del servidor.
No sueles escribirlo tu a mano como cliente.

Formato de prefix mas comun:

```text
:nick!user@host COMANDO ...
```

Otros formatos validos:

```text
:nick COMANDO ...
:servidor.local COMANDO ...
```

Ejemplos reales de lineas con prefix (salida del servidor):

```text
:ana!ana@localhost PRIVMSG #general :hola a todos
:irc.local 001 pepe :Welcome to the Internet Relay Network pepe
:irc.local 433 * pepe :Nickname is already in use
```

Ejemplo de linea que SI mandas tu (sin prefix):

```text
PRIVMSG #general :hola a todos
```

Si mandas prefix manualmente, muchos servidores lo ignoran o lo consideran invalido en comandos de cliente.

Ejemplo:

```text
PRIVMSG #general :hola equipo como va
```

- Comando: `PRIVMSG`
- Parametros: `#general`
- Trailing: `hola equipo como va`

## 3. Flujo minimo de registro (obligatorio)

Orden recomendado al conectar:

```text
PASS <password>
NICK <nick>
USER <user> 0 * :<realname>
```

Ejemplo:

```text
PASS 1234
NICK pepe
USER pepe 0 * :Pepe Lopez
```

Que significa cada campo de `USER`:
- `USER pepe 0 * :Pepe Lopez`
- `pepe` -> username
- `0` -> modo/flag historico (normalmente se deja 0)
- `*` -> unused (historico)
- `:Pepe Lopez` -> realname (puede llevar espacios por el `:`)

Cuando el registro es correcto, recibiras codigos de bienvenida `001` a `004`.

## 4. Comandos aceptados antes del registro

- `CAP` (se ignora)
- `PASS`
- `NICK`
- `USER`
- `QUIT`

Si envias otro comando sin estar registrado -> `451 You have not registered`.

## 5. Comandos principales (ya registrado)

### JOIN
```text
JOIN #canal
JOIN #canal <key>
```

### PART
```text
PART #canal
PART #canal :motivo de salida
```

### PRIVMSG
```text
PRIVMSG #canal :mensaje al canal
PRIVMSG nickDestino :mensaje privado
```

Ejemplos reales:

```text
PRIVMSG #general :hola equipo, reunion en 5 minutos
PRIVMSG bob :te paso el resumen ahora
```

Si olvidas `:` y hay espacios, solo se tomara la primera palabra como texto.

Mal:

```text
PRIVMSG #general hola equipo
```

Bien:

```text
PRIVMSG #general :hola equipo
```

### TOPIC
```text
TOPIC #canal
TOPIC #canal :nuevo topic
```

### MODE (canal)
```text
MODE #canal
MODE #canal +i
MODE #canal -i
MODE #canal +t
MODE #canal -t
MODE #canal +k clave
MODE #canal -k
MODE #canal +l 20
MODE #canal -l
MODE #canal +o nick
MODE #canal -o nick
```

### INVITE
```text
INVITE nick #canal
```

### KICK
```text
KICK #canal nick :motivo
```

### PING
```text
PING token123
```

### QUIT
```text
QUIT :me voy
```

### FILE (extension de este servidor)
```text
FILE SEND <nick> <filename> :<contenido>
FILE LIST
FILE GET <filename>
```

## 6. Codigos de respuesta frecuentes

## Exito / informacion
- `001` Welcome
- `002` Host info
- `003` Server created
- `004` Server/version info
- `324` Modos actuales del canal
- `331` No topic is set
- `332` Topic actual
- `341` Invite confirmado
- `353` Lista de nombres
- `366` Fin de lista de nombres

## Errores de cliente/comando
- `401` No such nick
- `403` No such channel
- `404` (usado en FILE GET cuando no existe archivo)
- `411` No recipient given (PRIVMSG)
- `412` No text to send
- `421` Unknown command
- `431` No nickname given
- `433` Nickname is already in use
- `441` They aren't on that channel
- `442` You're not on that channel
- `443` Is already on channel
- `451` You have not registered
- `461` Not enough parameters
- `462` You may not reregister
- `464` Password incorrect
- `471` Cannot join channel (+l)
- `473` Cannot join channel (+i)
- `475` Cannot join channel (+k)
- `482` You're not channel operator

## 7. Mini sesion de ejemplo

Cliente A:

```text
PASS 1234
NICK ana
USER ana 0 * :Ana User
JOIN #general
PRIVMSG #general :hola a todos
```

Cliente B:

```text
PASS 1234
NICK bob
USER bob 0 * :Bob User
JOIN #general
PRIVMSG ana :hola ana
```

Lineas tipicas que recibiria Ana desde el servidor:

```text
:bob!bob@localhost PRIVMSG ana :hola ana
```

## 8. Cosas que el servidor ignora

- `CAP` (capability negotiation)
- `WHO` (algunos clientes lo mandan automaticamente)

## 9. Consejo de prueba rapida

Si un comando parece no funcionar:
- Verifica que ya hiciste `PASS`, `NICK`, `USER`.
- Verifica que los canales empiecen por `#`.
- Para mensajes con espacios, usa `:trailing`.
