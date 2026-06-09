#!/bin/bash
# ================================================================
#               ARGENTUM ONLINE — SERVER LAUNCHER
#
#            Para cambiar el puerto, editá PORT abajo
# ================================================================

GREEN='\033[0;32m'
BLUE='\033[94m'
NC='\033[0m'

GAME_NAME="argentum"
PORT=8080

export ARGENTUM_SERVER_CONFIG_FILE="/etc/${GAME_NAME}/game_config.toml"
export ARGENTUM_DATA_DIR="/var/${GAME_NAME}"

echo -e "${BLUE}[Argentum] Iniciando servidor en el puerto ${PORT}...${NC}"

cd /usr/bin
./${GAME_NAME}_server "$PORT"

# Para correr con Valgrind, comenta la linea anterior y descomenta la siguiente:
# valgrind --leak-check=full --track-origins=yes --show-leak-kinds=all ./${GAME_NAME}_server "$PORT"
