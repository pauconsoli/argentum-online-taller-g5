#!/bin/bash

set -e

# Script para compilar y ejecutar con UndefinedBehaviorSanitizer (UBSAN)
# Detecta undefined behavior: división por cero, overflow, etc.

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}==> Compilando con UndefinedBehaviorSanitizer (UBSAN)...${NC}"
echo ""

# Verificar que clang++ esté disponible
if ! command -v clang++ &> /dev/null; then
    echo -e "${RED}Error: clang++ no está instalado${NC}"
    echo -e "Instalar: ${YELLOW}sudo apt install clang${NC}"
    exit 1
fi

# Compilar con UBSAN
cmake -S . -B build_ubsan \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_BUILD_TYPE=Debug \
    -DTALLER_ENABLE_UBSAN=ON

echo -e "${GREEN}✓${NC} CMake configurado con UBSAN"
echo ""

cmake --build build_ubsan

echo -e "${GREEN}✓${NC} Compilación completada"
echo ""

echo -e "${YELLOW}==> Ejecutando tests con UBSAN...${NC}"
echo ""

# Opciones UBSAN útiles:
# - halt_on_error=1: Detiene en el primer error
# - print_stacktrace=1: Imprime stack trace
# - verbosity=1: Output detallado
export UBSAN_OPTIONS="halt_on_error=1:print_stacktrace=1:verbosity=1"

if [ -f "./build_ubsan/taller_tests" ]; then
    ./build_ubsan/taller_tests
    echo -e "${GREEN}✓${NC} Tests completados sin errores de UBSAN"
else
    echo -e "${RED}Error: No se encontró ./build_ubsan/taller_tests${NC}"
    exit 1
fi
