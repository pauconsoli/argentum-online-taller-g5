#!/bin/bash

set -e

echo "==> Compilando tests en Debug..."

cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

echo ""
echo "==> Corriendo tests con Valgrind..."
echo ""

valgrind \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --verbose \
    --error-exitcode=1 \
    ./build/taller_tests