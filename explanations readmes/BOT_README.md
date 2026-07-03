# Bot de alcairc - Documentación

## Overview

El **Bot de alcairc** es una clase integrada en el servidor IRC que funciona como una entidad del servidor, **no como un cliente normal**. El bot se inicializa automáticamente al arrancar el servidor y puede responder a comandos dentro de los canales.

## Características

### ✅ Inicialización Automática
- El bot se crea cuando el servidor se inicia
- Se inicializa con el nombre "BotMaster" por defecto
- Está completamente integrado en la arquitectura del servidor

### ✅ Integración del Servidor
El bot recibe notificaciones de eventos del servidor:

1. **`onServerStart()`** - Se llama cuando el servidor inicia
2. **`onChannelCreated(Channel*)`** - Se llama cuando se crea un nuevo canal
3. **`onMessageReceived(channel, sender, message)`** - Se llama cuando alguien envía un mensaje en un canal

### ✅ Sistema de Comandos
El bot responde a comandos que empiezan con `!`:

| Comando | Descripción |
|---------|-------------|
| `!help` | Muestra todos los comandos disponibles |
| `!info` | Información del bot (nombre, versión, estado) |
| `!list` | Lista los comandos disponibles |
| `!welcome` | Mensaje de bienvenida |
| `!echo <texto>` | Repite el texto enviado |

## Ejemplo de Uso

### Compilar el servidor con el bot:
```bash
cd alcairc
make clean
make
```

### Ejecutar el servidor:
```bash
./ircserv 6667 password123
```

### Usar el bot desde un cliente IRC:
```
# Conectarse al canal
/join #test

# Ejecutar comandos del bot
/msg #test !help
/msg #test !info
/msg #test !echo Hola mundo
```

## Arquitectura del Bot

### Archivos principales:
- **Bot.hpp** - Declaración de la clase Bot
- **Bot.cpp** - Implementación del Bot
- **Server.hpp** - Forward declaration del Bot
- **Server.cpp** - Integración del Bot con el servidor

### Flujo de integración:

```
1. Server::Server()
   ├─ Crea instancia de Bot
   └─ Configura Bot con referencia al servidor

2. Server::run()
   ├─ Llama a bot->onServerStart()
   └─ Inicia el loop de eventos

3. Server::_cmdJoin()
   ├─ Detecta creación de nuevo canal
   └─ Llama a bot->onChannelCreated(channel)

4. Server::_cmdPrivmsg()
   ├─ Detecta mensajes en canales
   └─ Llama a bot->onMessageReceived(channel, sender, message)

5. Bot::onMessageReceived()
   ├─ Verifica si es un comando (empieza con '!')
   ├─ Si es comando, procesa con _processCommand()
   └─ Envía respuesta al canal con sendMessageToChannel()
```

## Extensiones Posibles

El bot puede extenderse fácilmente:

### 1. Agregar nuevos comandos:
```cpp
// En Bot.cpp, agregar en _processCommand():
else if (cmd == "tiempo")
{
    _handleTimeCommand(channel);
    return true;
}

// Implementar el handler:
void Bot::_handleTimeCommand(const std::string& channel)
{
    time_t now = time(0);
    sendMessageToChannel(channel, "Hora actual: " + std::string(ctime(&now)));
}
```

### 2. Respuestas automáticas:
```cpp
// En onMessageReceived(), detectar palabras clave:
if (message.find("hola") != std::string::npos)
{
    sendMessageToChannel(channel, "¡Hola " + sender + "!");
}
```

### 3. Almacenamiento de datos:
```cpp
// Agregar un mapa en Bot.hpp para guardar información:
std::map<std::string, std::string> _customData;

// Usar en comandos para guardar/recuperar datos
```

### 4. Moderación automática:
```cpp
// Detectar spam, palabras prohibidas, etc.
if (containsBadWord(message))
{
    // Notificar a moderadores o tomar acciones
}
```

## Diferencias con un cliente IRC tradicional

| Aspecto | Cliente Normal | Bot de alcairc |
|--------|--------|--------|
| **Arquitectura** | Proceso externo | Clase integrada en Server |
| **Inicialización** | Manual | Automática |
| **Acceso a datos** | A través de IRC | Acceso directo a objetos |
| **Rendimiento** | Requiere socket | Sin overhead de socket |
| **Complejidad** | Mayor (protocolo IRC) | Menor (integración directa) |
| **Control** | Limitado al protocolo | Completo |

## Nota importante sobre la implementación actual

La implementación actual del bot muestra el concepto de integración. Para un bot completamente funcional que envíe mensajes visibles a otros clientes, es necesario:

1. Crear una entidad "cliente bot" en la lista de clientes del servidor
2. O modificar el sistema de broadcast para incluir mensajes del bot
3. O crear un método especial en Channel para enviar mensajes del servidor

La base está implementada y lista para estas extensiones.
