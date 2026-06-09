#!/bin/bash
# ================================================================
#               ARGENTUM ONLINE — CLIENT LAUNCHER
#
#           Para cambiar host o puerto, editá HOST y PORT
# ================================================================

GREEN='\033[0;32m'
BLUE='\033[94m'
NC='\033[0m'

GAME_NAME="argentum"
HOST=localhost
PORT=8080

export ARGENTUM_CLIENT_CONFIG_FILE="/etc/${GAME_NAME}/game_config.toml"
export ARGENTUM_DATA_DIR="/var/${GAME_NAME}"

echo -e "${BLUE}[Argentum] Conectando al servidor ${HOST}:${PORT}...${NC}"

cd /usr/bin
./${GAME_NAME}_client "$HOST" "$PORT"
