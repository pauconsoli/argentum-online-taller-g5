#include "world_map_loader.h"

#include <stdexcept>
#include <string>
#include <utility>

#include <toml++/toml.hpp>

#include "server/world/cell.h"
#include "server/world/terrain_type.h"

static std::pair<TerrainType, bool> char_to_cell(char c) {
    switch (c) {
        case '.':
            return {TerrainType::GRASS, false};  // pasto solo
        case 'T':
            return {TerrainType::GRASS, true};  // árbol
        case ',':
            return {TerrainType::DIRT, false};
        case 's':
            return {TerrainType::SAND, false};
        case '~':
            return {TerrainType::WATER, true};
        case '#':
            return {TerrainType::STONE, true};

        default:
            throw std::runtime_error(std::string("Tile desconocido en el mapa: '") + c + "'");
    }
}

WorldMap WorldMapLoader::load(const std::string& config_path) {
    auto config = toml::parse_file(config_path);

    auto width_opt = config["map"]["width"].value<int>();  // si existe y es un int tiene valor, si
                                                           // no existe o es otro tipo está vacío
    auto height_opt = config["map"]["height"].value<int>();

    if (!width_opt || !height_opt)
        throw std::runtime_error("map.toml: falta 'width' o 'height' en [map]");

    int width = *width_opt;
    int height = *height_opt;

    const auto* rows = config["map"]["rows"].as_array();
    if (!rows)
        throw std::runtime_error("map.toml: falta el array 'rows' en [map]");
    if (static_cast<int>(rows->size()) != height)
        throw std::runtime_error("map.toml: cantidad de filas no coincide con 'height'");

    WorldMap map(width, height);

    for (int y = 0; y < height; y++) {
        auto row_opt = (*rows)[y].value<std::string>();
        if (!row_opt)
            throw std::runtime_error("map.toml: fila no es un string");

        const std::string& row = *row_opt;
        if (static_cast<int>(row.size()) != width)
            throw std::runtime_error("map.toml: longitud de fila no coincide con 'width'");

        for (int x = 0; x < width; x++) {
            auto [terrain, blocking] = char_to_cell(row[x]);
            map.set_cell({x, y}, Cell(terrain, blocking));
        }
    }

    return map;
}
