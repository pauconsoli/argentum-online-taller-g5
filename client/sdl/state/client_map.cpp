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

    // === Área de prueba de transiciones agua-arena (filas 14-32, cols 2-22) ===
    //
    // Caso A: lago 5×4 con borde de arena de 1 tile.
    //   Agua: cols 6-10, filas 17-20.
    //   Arena: cols 5-11, filas 16-21 (borde completo).
    //   Cubre: bordes N/S/E/W y las cuatro esquinas exteriores (agua en 2 lados adyacentes).
    set_rect(5, 16, 7, 6, TerrainType::SAND, false);  // anillo de arena
    set_rect(6, 17, 5, 4, TerrainType::WATER, true);  // lago interior
    //
    // Caso B: esquinas interiores aisladas (agua solo en diagonal).
    //   Cuatro WATER solitarios en diagonal de cuatro SAND solitarios.
    //   Ninguna celda WATER comparte lado con la SAND correspondiente.
    //
    //   NW: agua(14,14) → arena(15,15) → corner_nw=0°
    set_rect(14, 14, 1, 1, TerrainType::WATER, true);
    set_rect(15, 15, 1, 1, TerrainType::SAND, false);
    //   NE: agua(18,14) → arena(17,15) → corner_ne=90°
    set_rect(18, 14, 1, 1, TerrainType::WATER, true);
    set_rect(17, 15, 1, 1, TerrainType::SAND, false);
    //   SE: agua(18,24) → arena(17,23) → corner_se=180°
    set_rect(18, 24, 1, 1, TerrainType::WATER, true);
    set_rect(17, 23, 1, 1, TerrainType::SAND, false);
    //   SW: agua(14,24) → arena(15,23) → corner_sw=270°
    set_rect(14, 24, 1, 1, TerrainType::WATER, true);
    set_rect(15, 23, 1, 1, TerrainType::SAND, false);
    //
    // Caso C: tile completamente rodeado de agua (4 bordes).
    set_rect(20, 18, 3, 3, TerrainType::WATER, true);  // 3×3 agua
    set_rect(21, 19, 1, 1, TerrainType::SAND, false);  // arena central → N+S+E+W edges
    //
    // Caso D: arena en límite del mapa (fila 0, sin vecino norte).
    //   has_terrain(col,row-1,WATER) con row-1<0 debe retornar false sin crash.
    set_rect(20, 0, 1, 1, TerrainType::SAND, false);
    set_rect(20, 1, 1, 1, TerrainType::WATER, true);  // agua al sur → solo south edge

    // === Bloque de ciudad (filas 30-50, cols 30-55) ===
    // Piso de adoquines en el interior.
    set_rect(30, 30, 26, 21, TerrainType::CITY_FLOOR_2, false);
    // Paredes exteriores (CITY_WALL_1, overlay de 32×160 — 5 tiles de alto), bloqueantes.
    set_rect(30, 30, 26, 1, TerrainType::CITY_WALL_1, true);  // borde norte
    set_rect(30, 50, 26, 1, TerrainType::CITY_WALL_1, true);  // borde sur
    set_rect(30, 31, 1, 19, TerrainType::CITY_WALL_1, true);  // borde oeste
    set_rect(55, 31, 1, 19, TerrainType::CITY_WALL_1, true);  // borde este
    // Casas individuales dentro del bloque (CITY_WALL_2 usa overlay distinto).
    set_rect(33, 33, 3, 1, TerrainType::CITY_WALL_2, true);  // casa A — pared norte
    set_rect(33, 36, 3, 1, TerrainType::CITY_WALL_2, true);  // casa A — pared sur
    set_rect(33, 34, 1, 2, TerrainType::CITY_WALL_2, true);  // casa A — pared oeste
    set_rect(35, 34, 1, 2, TerrainType::CITY_WALL_2, true);  // casa A — pared este
    set_rect(42, 33, 4, 1, TerrainType::CITY_WALL_2, true);  // casa B — pared norte
    set_rect(42, 37, 4, 1, TerrainType::CITY_WALL_2, true);  // casa B — pared sur
    set_rect(42, 34, 1, 3, TerrainType::CITY_WALL_2, true);  // casa B — pared oeste
    set_rect(45, 34, 1, 3, TerrainType::CITY_WALL_2, true);  // casa B — pared este
    set_rect(33, 42, 5, 1, TerrainType::CITY_WALL_2, true);  // casa C — pared norte
    set_rect(33, 47, 5, 1, TerrainType::CITY_WALL_2, true);  // casa C — pared sur
    set_rect(33, 43, 1, 4, TerrainType::CITY_WALL_2, true);  // casa C — pared oeste
    set_rect(37, 43, 1, 4, TerrainType::CITY_WALL_2, true);  // casa C — pared este

    return ClientMap(W, H, std::move(cells));
}
