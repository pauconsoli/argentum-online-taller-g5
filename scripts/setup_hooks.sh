#!/bin/bash

set -e

# Colors para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}==> Configurando pre-commit hooks...${NC}"
echo ""

# Verificar si pip está instalado
if ! command -v pip &> /dev/null; then
    echo -e "${RED}Error: pip no está instalado${NC}"
    exit 1
fi

echo -e "${GREEN}✓${NC} pip encontrado"

# Instalar pre-commit si no está presente
if ! command -v pre-commit &> /dev/null; then
    echo -e "${YELLOW}→${NC} Instalando pre-commit..."
    pip install pre-commit
    echo -e "${GREEN}✓${NC} pre-commit instalado"
else
    echo -e "${GREEN}✓${NC} pre-commit ya instalado"
fi

# Instalar git hooks
echo -e "${YELLOW}→${NC} Instalando git hooks..."
pre-commit install
echo -e "${GREEN}✓${NC} Git hooks instalados"

# Verificar dependencias adicionales
echo ""
echo -e "${YELLOW}==> Verificando dependencias...${NC}"

# clang-format
if command -v clang-format &> /dev/null; then
    echo -e "${GREEN}✓${NC} clang-format instalado ($(clang-format --version | head -1))"
else
    echo -e "${RED}✗${NC} clang-format NO está instalado"
    echo -e "  → Instalar: ${YELLOW}sudo apt install clang-format${NC}"
fi

# cppcheck
if command -v cppcheck &> /dev/null; then
    echo -e "${GREEN}✓${NC} cppcheck instalado ($(cppcheck --version))"
else
    echo -e "${RED}✗${NC} cppcheck NO está instalado"
    echo -e "  → Instalar: ${YELLOW}sudo apt install cppcheck${NC}"
fi

# cpplint (Python)
if python3 -c "import cpplint" 2>/dev/null; then
    echo -e "${GREEN}✓${NC} cpplint instalado"
else
    echo -e "${RED}✗${NC} cpplint NO está instalado"
    echo -e "  → Instalar: ${YELLOW}pip install cpplint${NC}"
fi

# valgrind
if command -v valgrind &> /dev/null; then
    echo -e "${GREEN}✓${NC} valgrind instalado ($(valgrind --version | head -1))"
else
    echo -e "${RED}✗${NC} valgrind NO está instalado"
    echo -e "  → Instalar: ${YELLOW}sudo apt install valgrind${NC}"
fi

# clang (para ASAN/UBSAN)
if command -v clang++ &> /dev/null; then
    echo -e "${GREEN}✓${NC} clang++ instalado ($(clang++ --version | head -1))"
else
    echo -e "${YELLOW}⚠${NC} clang++ NO está instalado (necesario para ASAN/UBSAN)"
    echo -e "  → Instalar: ${YELLOW}sudo apt install clang${NC}"
fi

echo ""
echo -e "${GREEN}==> Configuración completada${NC}"
echo ""
echo "Scripts disponibles:"
echo "  • ${YELLOW}./scripts/setup_hooks.sh${NC} - Configurar git hooks (este script)"
echo "  • ${YELLOW}./scripts/run_valgrind.sh${NC} - Ejecutar tests con Valgrind"
echo "  • ${YELLOW}./scripts/run_asan.sh${NC} - Compilar y ejecutar con ASAN"
echo "  • ${YELLOW}./scripts/run_ubsan.sh${NC} - Compilar y ejecutar con UBSAN"
echo ""