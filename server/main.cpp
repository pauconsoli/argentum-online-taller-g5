#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include "loader/game_config_loader.h"
#include "loader/world_map_loader.h"
#include "server.h"
#include "world/world.h"

static constexpr const char* MAP_CONFIG_PATH = "common/config/map.toml";
static constexpr const char* GAME_CONFIG_PATH = "common/config/game_config.toml";

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << (argc > 0 ? argv[0] : "taller_server") << " <port>\n";
        return EXIT_FAILURE;
    }

    const std::string port = argv[1];

    try {
        GameConfigLoader::load(GAME_CONFIG_PATH);
        auto world = std::make_unique<World>(WorldMapLoader::load(MAP_CONFIG_PATH));
        Server server(port, std::move(world));
        server.run();
        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Error desconocido\n";
        return EXIT_FAILURE;
    }
}
