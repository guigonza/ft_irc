#!/bin/bash

################################################################################
#                                                                              #
#    test_bot.sh - Script de testing automatizado para bot IRC                #
#                                                                              #
#    Uso:                                                                      #
#      ./test_bot.sh                 # Compilar y testear versión bonus      #
#      ./test_bot.sh --simple        # Compilar y testear versión normal     #
#      ./test_bot.sh --help          # Mostrar esta ayuda                    #
#                                                                              #
################################################################################

set -e

PORT=6667
PASS="testpass"
HOST="localhost"
TEST_MODE="bonus"

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_header() {
    echo -e "${BLUE}===============================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}===============================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_info() {
    echo -e "${YELLOW}• $1${NC}"
}

show_help() {
    cat << EOF
USO: $0 [OPCIÓN]

OPCIONES:
  (sin argumento)    Compila y testea versión bonus con bot
  --simple           Compila y testea versión normal sin bot
  --help             Muestra esta ayuda
  --compile-only     Solo compila, no ejecuta tests
  --test-only        Solo ejecuta tests (requiere servidor activo)

EJEMPLOS:
  $0                  # Test completo con bot
  $0 --simple         # Test completo sin bot
  $0 --compile-only   # Solo compilar
  $0 --help           # Mostrar ayuda

EOF
}

compile_normal() {
    print_header "Compilando versión NORMAL (sin bot)"
    cd /home/carbon/irc
    make clean > /dev/null 2>&1
    if make all > /dev/null 2>&1; then
        print_success "Compilación exitosa: ircserv"
        ls -lh ircserv
    else
        print_error "Error en compilación"
        exit 1
    fi
}

compile_bonus() {
    print_header "Compilando versión BONUS (con bot)"
    cd /home/carbon/irc
    make clean > /dev/null 2>&1
    if make bonus > /dev/null 2>&1; then
        print_success "Compilación exitosa: ircserv_bonus"
        ls -lh ircserv_bonus
    else
        print_error "Error en compilación"
        exit 1
    fi
}

test_normal() {
    print_header "Testing versión NORMAL"
    
    # Crear archivo temporal para log
    LOGFILE=$(mktemp)
    
    # Iniciar servidor
    print_info "Iniciando servidor en puerto $PORT..."
    cd /home/carbon/irc
    timeout 30 ./ircserv $PORT $PASS > "$LOGFILE" 2>&1 &
    SERVER_PID=$!
    
    sleep 2
    
    # Test de conexión básica
    print_info "Test 1: Conexión básica"
    if echo -e "PASS $PASS\r\nNICK testuser\r\nUSER testuser 0 * :Test\r\nQUIT\r\n" | nc -w 1 $HOST $PORT > /dev/null 2>&1; then
        print_success "Cliente conectó correctamente"
    else
        print_error "No se pudo conectar al servidor"
    fi
    
    # Limpiar
    kill $SERVER_PID 2>/dev/null || true
    rm -f "$LOGFILE"
}

test_bonus() {
    print_header "Testing versión BONUS (con BOT)"
    
    # Crear archivo temporal para log
    LOGFILE=$(mktemp)
    
    # Iniciar servidor
    print_info "Iniciando servidor con bot en puerto $PORT..."
    cd /home/carbon/irc
    timeout 45 ./ircserv_bonus $PORT $PASS > "$LOGFILE" 2>&1 &
    SERVER_PID=$!
    
    sleep 3
    
    # Verificar que el bot se conectó
    print_info "Test 1: Verificar que bot está activo"
    if grep -q "Bot iniciado y conectado" "$LOGFILE"; then
        print_success "Bot conectado exitosamente"
    else
        print_error "Bot no se conectó"
    fi
    
    # Test de comando !ping
    print_info "Test 2: Comando !ping"
    OUTPUT=$(echo -e "PASS $PASS\r\nNICK testuser\r\nUSER testuser 0 * :Test\r\nJOIN #test\r\nPRIVMSG #test :!ping\r\nQUIT\r\n" | nc -w 2 $HOST $PORT 2>/dev/null)
    if echo "$OUTPUT" | grep -q "Pong!"; then
        print_success "!ping respondió correctamente"
    else
        print_error "!ping no respondió"
    fi
    
    # Test de comando !help
    print_info "Test 3: Comando !help"
    OUTPUT=$(echo -e "PASS $PASS\r\nNICK testuser\r\nUSER testuser 0 * :Test\r\nJOIN #test\r\nPRIVMSG #test :!help\r\nQUIT\r\n" | nc -w 2 $HOST $PORT 2>/dev/null)
    if echo "$OUTPUT" | grep -q "Comandos disponibles"; then
        print_success "!help respondió correctamente"
    else
        print_error "!help no respondió"
    fi
    
    # Test de comando !echo
    print_info "Test 4: Comando !echo"
    OUTPUT=$(echo -e "PASS $PASS\r\nNICK testuser\r\nUSER testuser 0 * :Test\r\nJOIN #test\r\nPRIVMSG #test :!echo Prueba\r\nQUIT\r\n" | nc -w 2 $HOST $PORT 2>/dev/null)
    if echo "$OUTPUT" | grep -q "Prueba"; then
        print_success "!echo respondió correctamente"
    else
        print_error "!echo no respondió"
    fi
    
    # Test de comando !info
    print_info "Test 5: Comando !info"
    OUTPUT=$(echo -e "PASS $PASS\r\nNICK testuser\r\nUSER testuser 0 * :Test\r\nJOIN #test\r\nPRIVMSG #test :!info\r\nQUIT\r\n" | nc -w 2 $HOST $PORT 2>/dev/null)
    if echo "$OUTPUT" | grep -q "ft_irc"; then
        print_success "!info respondió correctamente"
    else
        print_error "!info no respondió"
    fi
    
    # Test de comando !contador
    print_info "Test 6: Comando !contador"
    OUTPUT=$(echo -e "PASS $PASS\r\nNICK testuser\r\nUSER testuser 0 * :Test\r\nJOIN #test\r\nPRIVMSG #test :!contador\r\nQUIT\r\n" | nc -w 2 $HOST $PORT 2>/dev/null)
    if echo "$OUTPUT" | grep -q "Contador"; then
        print_success "!contador respondió correctamente"
    else
        print_error "!contador no respondió"
    fi
    
    # Test de comando !hora
    print_info "Test 7: Comando !hora"
    OUTPUT=$(echo -e "PASS $PASS\r\nNICK testuser\r\nUSER testuser 0 * :Test\r\nJOIN #test\r\nPRIVMSG #test :!hora\r\nQUIT\r\n" | nc -w 2 $HOST $PORT 2>/dev/null)
    if echo "$OUTPUT" | grep -qE "[0-9]{4}-[0-9]{2}-[0-9]{2}"; then
        print_success "!hora respondió correctamente"
    else
        print_error "!hora no respondió"
    fi
    
    # Limpiar
    kill $SERVER_PID 2>/dev/null || true
    rm -f "$LOGFILE"
}

main() {
    case "${1:-}" in
        --help)
            show_help
            exit 0
            ;;
        --simple)
            TEST_MODE="normal"
            compile_normal
            echo ""
            test_normal
            print_header "Testing COMPLETADO"
            ;;
        --compile-only)
            compile_bonus
            print_header "Compilación COMPLETADA"
            ;;
        --test-only)
            test_bonus
            print_header "Testing COMPLETADO"
            ;;
        *)
            compile_bonus
            echo ""
            test_bonus
            print_header "Testing COMPLETADO"
            ;;
    esac
}

main "$@"
