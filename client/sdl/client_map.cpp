#include "client_map.h"

#include <stdexcept>
#include <utility>

ClientMap::ClientMap(int width, int height, std::vector<MapCell> cells):
    width(width), height(height), cells(std::move(cells)) {}

const MapCell& ClientMap::at(int col, int row) const {
    if (col < 0 || col >= width || row < 0 || row >= height)
        throw std::out_of_range("ClientMap::at: coordenadas fuera de rango");
    return cells[row * width + col];
}

int ClientMap::get_width() const {
    return width;
}
int ClientMap::get_height() const {
    return height;
}

// TODO(chiaradelaurentis): reemplazar por construcción desde MapUpdate.
ClientMap build_sample_client_map() {
    const int W = 100;
    const int H = 100;

    // Fondo: todo pasto transitable.
    std::vector<MapCell> cells(W * H, {TerrainType::GRASS, false});

    auto set_rect = [&](int c0, int r0, int cw, int rh, TerrainType t, bool block) {
        for (int r = r0; r < r0 + rh; ++r)
            for (int c = c0; c < c0 + cw; ++c) cells[r * W + c] = {t, block};
    };

    // Obstáculo de arena cerca del spawn (col 1, fila 1): visible al arrancar.
    set_rect(3, 1, 3, 2, TerrainType::SAND, false);

    // Lago: 8 × 5 celdas, bloqueante.
    set_rect(4, 8, 8, 5, TerrainType::WATER, true);

    // Pared horizontal de piedra (fila 20, cols 8–20), bloqueante.
    set_rect(8, 20, 13, 1, TerrainType::STONE, true);

    // Pared vertical de piedra (col 24, filas 3–18), bloqueante.
    set_rect(24, 3, 1, 16, TerrainType::STONE, true);

    // Segunda zona de lago más al sur para dar variedad al mapa ampliado.
    set_rect(50, 40, 12, 8, TerrainType::WATER, true);

    // Franja de tierra (dirt) diagonal simulada: fila 60, cols 10–70.
    set_rect(10, 60, 60, 2, TerrainType::DIRT, false);

    return ClientMap(W, H, std::move(cells));
}
