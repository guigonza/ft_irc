# DCC File Transfer — Cheat Sheet práctica

## Cómo se envía un archivo (DCC SEND)

El emisor manda un `PRIVMSG` normal con el contenido CTCP:

```
PRIVMSG <receptor> :\x01DCC SEND <archivo> <ip_entera> <puerto> <tamaño>\x01
```

### Ejemplo real

Alice (127.0.0.1) quiere enviar `f.txt` (15 bytes) a Bob por puerto 5001:

```
PRIVMSG Bob :\x01DCC SEND f.txt 2130706433 5001 15\x01
```

---

## Qué hace tu servidor al recibirlo

**1. Detecta** que el `PRIVMSG` empieza con `\x01` (byte 0x01 = CTCP)

**2. Parsea** el mensaje con `FileHandler::parseDCCSend()` y extrae:

| Campo | Valor |
|---|---|
| filename | `f.txt` |
| ip | `2130706433` (127.0.0.1) |
| port | `5001` |
| size | `15` |

**3. Loguea** en consola del servidor:
```
[DCC SEND] Alice -> Bob "f.txt" 15 bytes (port 5001)
```

**4. Reenvía** el PRIVMSG idéntico a Bob:
```
:Alice!a@127.0.0.1 PRIVMSG Bob :\x01DCC SEND f.txt 2130706433 5001 15\x01
```

**5. Su trabajo termina ahí.** El cliente de Bob (HexChat/mIRC) ve el `\x01DCC SEND` y ofrece al usuario descargar. La transferencia es P2P.

---

## Cómo se prueba con netcat

### Terminal 1 — Servidor
```
./ircserv 6667 pass
```

### Terminal 2 — Alice (abre puerto y ofrece archivo)
```bash
# Primero: tener el archivo listo y un puerto escuchando
echo "Hola mundo!" > f.txt
nc -lvp 5001 < f.txt &   # Alice escucha para la futura conexion P2P

# Despues: conectar al IRC y enviar el DCC offer
nc localhost 6667
PASS pass
NICK Alice
USER a 0 * :Alice
PRIVMSG Bob :\x01DCC SEND f.txt 2130706433 5001 15\x01
```

### Terminal 3 — Bob (recibe el DCC)
```bash
nc localhost 6667
PASS pass
NICK Bob
USER b 0 * :Bob
# Veras llegar: :Alice!a@127.0.0.1 PRIVMSG Bob :\x01DCC SEND f.txt ...
```

### Terminal 4 — Simular que Bob acepta (conecta directo a Alice)
```bash
nc 127.0.0.1 5001 > f_recibido.txt
```

---

## Notas rápidas

- `\x01` = byte literal 0x01, no la cadena `"\x01"`. En netcat se escribe con `Ctrl+V Ctrl+A` o con `printf '\x01'`
- El servidor **nunca toca el archivo**, solo reenvía el mensaje de texto
- No hay comando `FILE` — se eliminó, no existe en IRC real
- La IP se codifica como entero 32 bits big-endian: `127.0.0.1` → `2130706433`

### Decodificar IP rápido

```
127.0.0.1 = (127<<24) + (0<<16) + (0<<8) + 1 = 2130706433
192.168.1.1 = (192<<24) + (168<<16) + (1<<8) + 1 = 3232235777
```
