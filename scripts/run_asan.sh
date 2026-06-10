#!/bin/bash

set -e

# Script para compilar y ejecutar con AddressSanitizer (ASAN)
# Detecta memory leaks, buffer overflows, use-after-free, etc.

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}==> Compilando con AddressSanitizer (ASAN)...${NC}"
echo ""

# Verificar que clang++ esté disponible
if ! command -v clang++ &> /dev/null; then
    echo -e "${RED}Error: clang++ no está instalado${NC}"
    echo -e "Instalar: ${YELLOW}sudo apt install clang${NC}"
    exit 1
fi

# Compilar con ASAN
cmake -S . -B build_asan \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_BUILD_TYPE=Debug \
    -DTALLER_ENABLE_ASAN=ON

echo -e "${GREEN}✓${NC} CMake configurado con ASAN"
echo ""

cmake --build build_asan

echo -e "${GREEN}✓${NC} Compilación completada"
echo ""

echo -e "${YELLOW}==> Ejecutando tests con ASAN...${NC}"
echo ""

# Opciones ASAN útiles:
# - halt_on_error=1: Detiene en el primer error
# - verbosity=1: Output detallado
# - log_path=asan.log: Logs en archivo
export ASAN_OPTIONS="halt_on_error=1:verbosity=1"

if [ -f "./build_asan/argentum_tests" ]; then
    ./build_asan/argentum_tests
    echo -e "${GREEN}✓${NC} Tests completados sin errores de ASAN"
else
    echo -e "${RED}Error: No se encontró ./build_asan/argentum_tests${NC}"
    exit 1
fi
