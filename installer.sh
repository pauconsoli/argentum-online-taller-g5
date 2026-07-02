#!/bin/bash
set -e
set -o pipefail

# =============================================
#                 Color codes
# =============================================

GREEN='\033[0;32m'
BLUE='\033[94m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'  # No color

# =============================================
#                  Config
# =============================================

GAME_NAME="argentum"
REPO_URL="https://github.com/pauconsoli/argentum-online-taller-g5.git"
BRANCH="main"
BUILD_DIR="/tmp/${GAME_NAME}_build"
BIN_DIR="/usr/bin"
CONFIG_DIR="/etc/${GAME_NAME}"
DATA_DIR="/var/${GAME_NAME}"

if [[ -n "$SUDO_USER" ]]; then
    USER_HOME=$(getent passwd "$SUDO_USER" | cut -d: -f6)
    DESKTOP_DIR=$(sudo -u "$SUDO_USER" xdg-user-dir DESKTOP 2>/dev/null)
else
    USER_HOME=$HOME
    DESKTOP_DIR=$(xdg-user-dir DESKTOP 2>/dev/null)
fi

if [[ -z "$DESKTOP_DIR" ]]; then
    DESKTOP_DIR="$USER_HOME/Desktop"
fi

# =============================================
#                  Helpers
# =============================================

print_step() { echo -e "\n${BLUE}==> $1${NC}"; }
print_ok()   { echo -e "${GREEN}[OK] $1${NC}"; }
print_warn() { echo -e "${YELLOW}[WARN] $1${NC}"; }
die()        { echo -e "${RED}[ERROR] $1${NC}" >&2; exit 1; }

require_sudo() {
    if [[ $EUID -ne 0 ]]; then
        die "Este installer necesita permisos de superusuario. Ejecutalo con: sudo bash installer.sh"
    fi
}

# =============================================
#               Verificaciones
# =============================================

require_sudo

echo -e "${BLUE}"
echo "  ╔═══════════════════════════════════════════════╗"
echo "  ║        Argentum Online (G5) — Installer       ║"
echo "  ╚═══════════════════════════════════════════════╝"
echo -e "${NC}"

# =============================================
#         Actualizar repositorios apt
# =============================================

print_step "Actualizando lista de paquetes..."
apt-get update -qq || print_warn "Algunos repositorios fallaron al actualizar. Continuando de todos modos..."
print_ok "Proceso de actualización finalizado"

# =============================================
#      Instalar dependencias del sistema
# =============================================
print_step "Instalando dependencias del sistema..."

PACKAGES=(
    # Herramientas base
    git
    make
    cmake
    build-essential
    pkg-config
    unzip
    zip
    curl
    ca-certificates

    # SDL2
    libsdl2-dev
    libsdl2-image-dev
    libsdl2-ttf-dev
    libsdl2-mixer-dev
    libopus-dev
    libopusfile-dev
    libxmp-dev
    libfluidsynth-dev
    fluidsynth
    libwavpack1
    libwavpack-dev
    libfreetype-dev
    wavpack

    # Qt5
    qtbase5-dev
    qttools5-dev
    qttools5-dev-tools

    # GoogleTest (unit tests del protocolo)
    libgtest-dev
    libgmock-dev

    # TOML parser — toml++ se usa header-only, pero por si el proyecto
    # lo instala como paquete del sistema:
    # libtomlplusplus-dev   # descomenta si tu distro lo tiene

    # Otras
    libssl-dev
    libpthread-stubs0-dev
    valgrind
    xdg-utils        
)

apt-get install -y "${PACKAGES[@]}"
print_ok "Dependencias instaladas"

# =============================================
#      Clonar / actualizar el repositorio
# =============================================

print_step "Clonando el repositorio desde GitHub..."

if [[ -d "$BUILD_DIR/.git" ]]; then
    print_warn "El directorio $BUILD_DIR ya existe. Actualizando..."
    git -C "$BUILD_DIR" fetch origin "$BRANCH"
    git -C "$BUILD_DIR" reset --hard "origin/$BRANCH"
else
    git clone --depth=1 --branch "$BRANCH" "$REPO_URL" "$BUILD_DIR"
fi
print_ok "Repositorio listo en $BUILD_DIR"

# =============================================
#            Compilar con CMake
# =============================================

print_step "Compilando el proyecto ..."

CMAKE_BUILD_DIR="$BUILD_DIR/build"
mkdir -p "$CMAKE_BUILD_DIR"
cmake -S "$BUILD_DIR" -B "$CMAKE_BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release

cmake --build "$CMAKE_BUILD_DIR"
print_ok "Compilación exitosa"

# =============================================
#            Correr unit tests
# =============================================

print_step "Corriendo unit tests..."
TEST_BIN="$CMAKE_BUILD_DIR/${GAME_NAME}_tests"
if [[ -f "$TEST_BIN" ]]; then
    if "$TEST_BIN"; then
        print_ok "Todos los tests pasaron"
    else
        print_warn "Algunos tests fallaron — el instalador continúa de todas formas"
    fi
else
    print_warn "No se encontró el binario de tests en $TEST_BIN. Saltando pruebas."
fi

# =============================================
#         Crear directorios destino
# =============================================
print_step "Creando directorios de instalación..."
mkdir -p "$CONFIG_DIR"
mkdir -p "$DATA_DIR"
if [[ -n "$SUDO_USER" ]]; then
    chown "$SUDO_USER:$SUDO_USER" "$DATA_DIR"
fi
if [[ ! -d "$DESKTOP_DIR" ]]; then
    mkdir -p "$DESKTOP_DIR"
    if [[ -n "$SUDO_USER" ]]; then
        chown "$SUDO_USER:$SUDO_USER" "$DESKTOP_DIR"
    fi
fi
print_ok "Directorios creados"

# =============================================
#       Instalar binarios en /usr/bin
# =============================================
print_step "Instalando binarios en $BIN_DIR..."

# Ejecutables
SERVER_BIN="$CMAKE_BUILD_DIR/${GAME_NAME}_server"
CLIENT_BIN="$CMAKE_BUILD_DIR/${GAME_NAME}_client_qt"

[[ -f "$SERVER_BIN" ]] || die "No se encontró el binario del servidor en $SERVER_BIN"
[[ -f "$CLIENT_BIN" ]] || die "No se encontró el binario del cliente en $CLIENT_BIN"

install -m 755 "$SERVER_BIN" "$BIN_DIR/${GAME_NAME}_server"
install -m 755 "$CLIENT_BIN" "$BIN_DIR/${GAME_NAME}_client_qt"

print_ok "Binarios instalados en $BIN_DIR"

# =========================================================
#    Instalar archivos de configuración en /etc/<juego>
# =========================================================

print_step "Copiando archivos de configuración a $CONFIG_DIR..."

CONFIG_SRC="$BUILD_DIR/common/config"
if [[ -d "$CONFIG_SRC" ]]; then
    cp -r "$CONFIG_SRC"/. "$CONFIG_DIR/"
    print_ok "Configuración copiada desde $CONFIG_SRC"
else
    print_warn "No se encontró $CONFIG_SRC — saltando copia de configs"
fi

# =======================================================
#          Instalar assets en /var/<juego>
# =======================================================
print_step "Copiando assets a $DATA_DIR..."

ASSETS_SRC="$BUILD_DIR/assets"
if [[ -d "$ASSETS_SRC" ]]; then
    cp -r "$ASSETS_SRC"/. "$DATA_DIR/"
    print_ok "Assets copiados desde $ASSETS_SRC"
else
    print_warn "No se encontró $ASSETS_SRC — saltando copia de assets"
fi

# ================================================
#    Crear server.sh y client.sh en el Desktop
# ================================================
print_step "Copiando launchers al escritorio ($DESKTOP_DIR)..."

cp "$BUILD_DIR/server.sh" "$DESKTOP_DIR/server.sh"
cp "$BUILD_DIR/client.sh" "$DESKTOP_DIR/client.sh"
chmod +x "$DESKTOP_DIR/server.sh"
chmod +x "$DESKTOP_DIR/client.sh"

if [[ -n "$SUDO_USER" ]]; then
    chown "$SUDO_USER:$SUDO_USER" "$DESKTOP_DIR/server.sh"
    chown "$SUDO_USER:$SUDO_USER" "$DESKTOP_DIR/client.sh"
fi

print_ok "Launchers instalados en $DESKTOP_DIR"

# =============================================
#      Limpiar build temporal (opcional)
# =============================================

print_step "Limpiando archivos temporales..."
rm -rf "$BUILD_DIR"
print_ok "Limpieza completa"

# =============================================
#               Resumen final
# =============================================

STR_TITLE="¡Instalación de Argentum Online completada!"
STR_BIN="Binarios   → $BIN_DIR/${GAME_NAME}_{server,client_qt}"
STR_CONF="Configs    → $CONFIG_DIR"
STR_DATA="Assets     → $DATA_DIR"
STR_LAUNCH="Launchers  → $DESKTOP_DIR/{server,client}.sh"
STR_PLAY="Para jugar:"
STR_PLAY1="  1. Abrí una terminal y ejecutá ./server.sh"
STR_PLAY2="  2. Abrí otra terminal y ejecutá ./client.sh"

MAX_LEN=0
for str in "$STR_TITLE" "$STR_BIN" "$STR_CONF" "$STR_DATA" "$STR_LAUNCH" "$STR_PLAY" "$STR_PLAY1" "$STR_PLAY2"; do
    if [[ ${#str} -gt $MAX_LEN ]]; then
        MAX_LEN=${#str}
    fi
done

MAX_LEN=$((MAX_LEN + 4))

BORDER=$(printf '%*s' "$MAX_LEN" '')
BORDER=${BORDER// /═}

print_row() {
    local str="$1"
    local pad=$(( MAX_LEN - ${#str} - 2 ))
    printf "║  %s%*s║\n" "$str" "$pad" ""
}

echo ""
echo -e "${GREEN}╔${BORDER}╗"
print_row "$STR_TITLE"
echo -e "╠${BORDER}╣"
print_row "$STR_BIN"
print_row "$STR_CONF"
print_row "$STR_DATA"
print_row "$STR_LAUNCH"
echo -e "╠${BORDER}╣"
print_row "$STR_PLAY"
print_row "$STR_PLAY1"
print_row "$STR_PLAY2"
echo -e "╚${BORDER}╝${NC}"
echo ""