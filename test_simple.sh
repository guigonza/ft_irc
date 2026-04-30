#!/bin/bash

# Simple bot test
PORT=6669
PASS="testpass"

echo "[TEST] Starting IRC server with bot..."
./ircserv_bonus $PORT $PASS > /tmp/server.log 2>&1 &
SERVER_PID=$!
sleep 2

echo "[TEST] Connecting client and sending commands..."
(
  sleep 1
  echo "PASS $PASS"
  echo "NICK client1"
  echo "USER client1 0 * :Client1"
  sleep 1
  echo "JOIN #test"
  sleep 1
  echo "PRIVMSG #test :!ping"
  sleep 1
  echo "PRIVMSG #test :!help"
  sleep 1
  echo "PRIVMSG #test :!echo hello world"
  sleep 2
  echo "QUIT"
) | nc -w 10 localhost $PORT > /tmp/client.log 2>&1

sleep 2
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null

echo ""
echo "=== SERVER LOG ==="
grep -E "(Bot|cmd=PRIVMSG|Autenticado)" /tmp/server.log | tail -20

echo ""
echo "=== CLIENT RESPONSE ==="
cat /tmp/client.log | tail -20

echo ""
echo "[TEST] Done"
