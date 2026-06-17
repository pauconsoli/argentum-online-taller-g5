#include "world_map_loader.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include <toml++/toml.hpp>

#include "common/world/terrain_type.h"
#include "server/world/cell.h"
#include "server/world/city.h"
#include "server/world/dungeon.h"

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
        case 'W':
            return {TerrainType::WOOD, false};
        case 'd':
            return {TerrainType::DUNGEON_FLOOR_1, false};
        case 'D':
            return {TerrainType::DUNGEON_WALL_1, true};
        case 'E':
            return {TerrainType::DUNGEON_ENTRANCE_1, false};
        case '*':
            return {TerrainType::DUNGEON_FLOOR_2, false};
        case 'P':
            return {TerrainType::DUNGEON_WALL_2, true};
        case '=':
        case '<':
            return {TerrainType::DUNGEON_ENTRANCE_2, false};
        case '-':
            return {TerrainType::CITY_FLOOR_1, false};
        case 'C':
            return {TerrainType::CITY_WALL_1, true};
        case ':':
            return {TerrainType::CITY_FLOOR_2, false};
        case 'A':
            return {TerrainType::CITY_WALL_2, true};
        default:
            throw std::runtime_error(std::string("Tile desconocido en el mapa: '") + c + "'");
    }
}

WorldMap WorldMapLoader::load(const std::string& config_path) {
    auto config = toml::parse_file(config_path);

    // MAP DIMENSIONS

    auto width_opt = config["map"]["width"].value<int>();  // si existe y es un int tiene valor, si
                                                           // no existe o es otro tipo está vacío
    auto height_opt = config["map"]["height"].value<int>();

    if (!width_opt || !height_opt)
        throw std::runtime_error("map.toml: falta 'width' o 'height' en [map]");

    int width = *width_opt;
    int height = *height_opt;

    // MAP CELLS

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

    // ZONES

    if (const auto* zones_arr = config["zones"].as_array()) {
        for (const auto& elem : *zones_arr) {
            const auto* zone_table = elem.as_table();
            if (!zone_table)
                continue;

            auto type_z = (*zone_table)["type"].value<std::string>();
            auto name_z = (*zone_table)["name"].value<std::string>();
            auto x_z = (*zone_table)["x"].value<int>();
            auto y_z = (*zone_table)["y"].value<int>();
            auto w_z = (*zone_table)["width"].value<int>();
            auto h_z = (*zone_table)["height"].value<int>();

            if (!type_z || !name_z || !x_z || !y_z || !w_z || !h_z) {
                throw std::runtime_error(
                    "map.toml: faltan atributos básicos en una de las definiciones de [[zones]]");
            }

            if (*type_z == "dungeon") {
                auto ent_x = (*zone_table)["entrance_x"].value<int>();
                auto ent_y = (*zone_table)["entrance_y"].value<int>();
                if (!ent_x || !ent_y)
                    throw std::runtime_error("map.toml: faltan entrance_x o entrance_y en dungeon");

                map.add_dungeon(std::make_unique<Dungeon>(*name_z, *ent_x, *ent_y), *x_z, *y_z,
                                *w_z, *h_z);
            } else if (*type_z == "city") {
                auto priest_x = (*zone_table)["priest_x"].value<int>();
                auto priest_y = (*zone_table)["priest_y"].value<int>();
                auto merchant_x = (*zone_table)["merchant_x"].value<int>();
                auto merchant_y = (*zone_table)["merchant_y"].value<int>();
                auto banker_x = (*zone_table)["banker_x"].value<int>();
                auto banker_y = (*zone_table)["banker_y"].value<int>();

                if (!priest_x || !priest_y || !merchant_x || !merchant_y || !banker_x ||
                    !banker_y) {
                    throw std::runtime_error("map.toml: faltan posiciones de NPCs en city");
                }

                map.add_city(std::make_unique<City>(*name_z, Position{*priest_x, *priest_y},
                                                    Position{*merchant_x, *merchant_y},
                                                    Position{*banker_x, *banker_y}),
                             *x_z, *y_z, *w_z, *h_z);
            } else {
                throw std::runtime_error("map.toml: tipo de zona desconocido '" + *type_z + "'");
            }
        }
    }

    return map;
}
