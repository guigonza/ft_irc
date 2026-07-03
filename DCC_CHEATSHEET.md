# DCC File Transfer — Cheat Sheet

## ¿Cómo funciona?

El servidor **solo relayea el handshake**. La transferencia es P2P entre clientes.

### Flujo

```
Alice (HexChat)                ft_irc                    Bob (HexChat)
     │                            │                           │
     │  Abre puerto local 5001    │                           │
     │                            │                           │
     │  PRIVMSG Bob :\x01DCC      │                           │
     │  SEND f.txt 2130706433     │                           │
     │  5001 9\x01                │                           │
     │───────────────────────────>│                           │
     │                            │  [DCC SEND] Alice -> Bob  │
     │                            │  "f.txt" 9 bytes (5001)   │
     │                            │                           │
     │                            │  PRIVMSG Alice :\x01DCC   │
     │                            │  SEND f.txt 2130706433    │
     │                            │  5001 9\x01               │
     │                            │──────────────────────────>│
     │                            │                           │
     │                            │  (servidor no participa)  │
     │<═══════ connect(P2P) ═════════════════════════════════>│
     │          Transferencia directa                         │
```

---

## Parseo del mensaje DCC SEND

### `isDCCMessage(trailing)`

Detecta si el trailing de un PRIVMSG empieza con `\x01DCC `:

| trailing | Resultado |
|---|---|
| `\x01DCC SEND f.txt 2130706433 5001 9\x01` | ✅ true |
| `\x01DCC RESUME f.txt 2130706433 5001 9\x01` | ✅ true |
| `hola que tal` | ❌ false |
| `\x01VERSION\x01` | ❌ false (no es DCC) |

### `parseDCCSend(trailing, fname, ip, port, size)`

Desglose byte a byte del trailing:

```
Índice:  0   1 2 3 4   5 6 7 8 9   10 11 12 13 14 15 ...
        \x01 D C C   S E N D   f  .  t  x  t     2  1 ...
              ↑ "DCC " ↑ "SEND " ↑ "f.txt 2130706433 5001 9\x01"
                                               substr(10)
```

1. `substr(10)` → extrae todo desde byte 10: `"f.txt 2130706433 5001 9\x01"`
2. Si último byte es `\x01`, se elimina → `"f.txt 2130706433 5001 9"`
3. `istringstream` separa por espacios:

| Orden | Token | Variable | Valor |
|---|---|---|---|
| 1º | `f.txt` | `outFilename` | `"f.txt"` |
| 2º | `2130706433` | `outIp` | `2130706433` = `127.0.0.1` |
| 3º | `5001` | `outPort` | `5001` (casteado a `unsigned short`) |
| 4º | `9` | `outSize` | `9` |

### Código en Server.cpp

```cpp
// Se ejecuta ANTES del if-else de comandos
// No tiene return → _cmdPrivmsg se ejecuta igual despues
if (command == "PRIVMSG" && !trailing.empty()
    && static_cast<unsigned char>(trailing[0]) == 0x01) {

    std::string fname;
    unsigned long ip, sz;
    unsigned short port;
    if (FileHandler::parseDCCSend(trailing, fname, ip, port, sz))
    {
        std::cout << "[DCC SEND] " << client.getNick()
                  << " -> " << params[0]
                  << " \"" << fname << "\" " << sz << " bytes"
                  << " (port " << port << ")" << std::endl;
    }
}
```

---

## Cómo probar con HexChat

### 1. Configurar servidor
- HexChat → Redes → Añadir
- Nombre: `ft_irc`
- Servidores: `localhost/6667`
- Contraseña: `pass`

### 2. Conectar dos instancias
- **Alice**: conectar normalmente
- **Bob**: `hexchat --existing` o abrir otra ventana, nick `Bob`

### 3. Enviar archivo
- Alice escribe: `/dcc send Bob /ruta/al/archivo.txt`
- Bob ve ventana "Archivo entrante de Alice"
- Bob acepta → transferencia P2P

### 4. Ver raw del DCC
- Settings → Advanced → Logging → marcar "Raw CTCP messages"
- O enviar manualmente: `/raw PRIVMSG Bob :\x01DCC SEND f.txt 2130706433 5001 9\x01`

---

## Cómo probar con netcat

### Terminal 1 — Servidor
```bash
cd /home/alejandro/dev/42Universe/42Proyects/IRC-guille
make re && ./ircserv 6667 pass
```

### Terminal 2 — Alice (crea archivo y abre puerto P2P)
```bash
echo "Hola DCC!" > /tmp/f.txt     # 9 bytes
nc -lvp 5001 < /tmp/f.txt         # espera conexion P2P
```

### Terminal 3 — Alice conecta al IRC y ofrece el DCC
```bash
nc localhost 6667
PASS pass
NICK Alice
USER a 0 * :Alice
PRIVMSG Bob :\x01DCC SEND f.txt 2130706433 5001 9\x01
```

Nota: el byte `\x01` se escribe como `Ctrl+V Ctrl+A` en nc, o con `printf`:
```bash
printf 'PASS pass\r\nNICK Alice\r\nUSER a 0 * :Alice\r\nPRIVMSG Bob :\x01DCC SEND f.txt 2130706433 5001 9\x01\r\n' | nc -q 0.5 localhost 6667
```

### Terminal 4 — Bob conecta y recibe el DCC
```bash
nc localhost 6667
PASS pass
NICK Bob
USER b 0 * :Bob
```

Verás llegar:
```
:Alice!a@127.0.0.1 PRIVMSG Bob :\x01DCC SEND f.txt 2130706433 5001 9\x01
```

### Terminal 5 — Bob simula aceptar (conecta directo a Alice)
```bash
nc 127.0.0.1 5001 > /tmp/f_recibido.txt
diff /tmp/f.txt /tmp/f_recibido.txt   # sin salida = iguales
```

### Log esperado en servidor
```
[DCC SEND] Alice -> Bob "f.txt" 9 bytes (port 5001)
```

---

## Decodificar IP (entero → ip con puntos)

```
127.0.0.1 → (127<<24) + (0<<16) + (0<<8) + 1 = 2130706433
192.168.1.1 → (192<<24) + (168<<16) + (1<<8) + 1 = 3232235777
```

### En C++
```cpp
unsigned long ip = 2130706433;
unsigned char byte1 = (ip >> 24) & 0xFF;  // 127
unsigned char byte2 = (ip >> 16) & 0xFF;  // 0
unsigned char byte3 = (ip >>  8) & 0xFF;  // 0
unsigned char byte4 =  ip        & 0xFF;  // 1
```
