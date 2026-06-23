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
