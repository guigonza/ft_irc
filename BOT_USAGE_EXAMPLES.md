# Ejemplos de Uso del Bot de alcairc

## 1. Compilación y Ejecución Básica

```bash
# Compilar el servidor con el bot
cd alcairc
make clean
make

# Ejecutar el servidor
./ircserv 6667 mipassword
```

Una vez ejecutado, verás en la consola:
```
[Server] Listening on port 6667
[Bot] Created bot 'BotMaster'
[Bot] Server started! Bot 'BotMaster' is online
```

## 2. Conectar Clientes y Usar Comandos del Bot

### Usando `nc` (netcat):

```bash
# Abrir una nueva terminal
nc localhost 6667
```

Luego escribe los siguientes comandos IRC:
```
PASS mipassword
NICK usuario1
USER usuario1 0 * :Usuario Uno
JOIN #test
PRIVMSG #test :!help
PRIVMSG #test :!info
PRIVMSG #test :!echo Probando el bot
QUIT
```

### Usando `telnet`:

```bash
telnet localhost 6667
```

Mismos comandos que con `nc`.

## 3. Respuestas del Bot

El bot responderá automáticamente a los comandos. Verás mensajes como:

```
:BotMaster PRIVMSG #test :Available commands:
:BotMaster PRIVMSG #test :  !help     - Show this help message
:BotMaster PRIVMSG #test :  !info     - Show bot information
:BotMaster PRIVMSG #test :  !list     - List available commands
:BotMaster PRIVMSG #test :  !welcome  - Welcome message
:BotMaster PRIVMSG #test :  !echo <text> - Echo back a message
```

## 4. Flujo de Ejecución Detallado

### Cuando se crea un canal:
1. Cliente ejecuta `/join #canal`
2. Servidor llama a `bot->onChannelCreated(channel)`
3. Bot envía un mensaje de bienvenida automáticamente

### Cuando se envía un mensaje con `!`:
1. Cliente envía `PRIVMSG #canal :!comando args`
2. Servidor llama a `bot->onMessageReceived(channel, sender, message)`
3. Bot comprueba si comienza con `!`
4. Bot procesa el comando correspondiente
5. Bot envía la respuesta al canal mediante `sendMessageToChannel()`

## 5. Ejemplo Completo en una Línea

Para una prueba rápida sin múltiples comandos:

```bash
# Terminal 1: Iniciar servidor
./ircserv 6667 password

# Terminal 2: Conectar cliente
echo -e "PASS password\nNICK bot_user\nUSER bot_user 0 * :Bot User\nJOIN #test\nPRIVMSG #test :!help\nQUIT" | nc localhost 6667
```

## 6. Logs y Depuración

En la consola del servidor verás:

```
[Bot] Created bot 'BotMaster'
[Bot] Server started! Bot 'BotMaster' is online
[Bot] New channel created: #test
[Bot] Sent to #test: Welcome to #test! I'm BotMaster, type !help for commands.
[Bot] Processing command 'help' from usuario1 in #test
[Bot] Sent to #test: Available commands:
...
```

## 7. Modificar el Bot

Para agregar nuevos comandos, edita `Bot.cpp`:

### Ejemplo: Agregar comando `!hora`

En `Bot.hpp`, agregar en los private methods:
```cpp
void _handleTimeCommand(Channel* channel);
```

En `Bot.cpp`, en `_processCommand()`:
```cpp
else if (cmd == "time")
{
    _handleTimeCommand(channel);
    return true;
}
```

Implementar:
```cpp
void Bot::_handleTimeCommand(Channel* channel)
{
    time_t now = time(0);
    char buffer[26];
    ctime_r(&now, buffer);
    sendMessageToChannel(channel, "Hora actual: " + std::string(buffer));
}
```

## 8. Características del Bot

✅ **Inicialización automática** - Se crea al iniciar el servidor
✅ **Sin cliente externo** - Está integrado en el servidor
✅ **Responde a comandos** - Procesa comandos con `!`
✅ **Envía mensajes reales** - Los mensajes son visibles a todos
✅ **Extensible** - Fácil agregar nuevos comandos
✅ **Notificaciones de eventos** - Se entera de canales nuevos y mensajes

## 9. Limitaciones Actuales

⚠️ El bot envía mensajes a través del sistema de broadcast del canal
⚠️ No se puede enviar mensajes privados completamente (se registran en consola)
⚠️ No almacena datos entre sesiones
⚠️ No tiene sistema de permisos avanzado

Estas pueden mejorarse modificando la clase Bot según necesidades.
