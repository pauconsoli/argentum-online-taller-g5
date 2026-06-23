#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include "loader/game_config_loader.h"
#include "loader/item_registry_loader.h"
#include "loader/npc_registry_loader.h"
#include "loader/world_item_loader.h"
#include "loader/world_map_loader.h"
#include "server.h"
#include "world/world.h"

static constexpr const char* DEFAULT_MAP_CONFIG_PATH = "common/config/map.toml";
static constexpr const char* DEFAULT_GAME_CONFIG_PATH = "common/config/game_config.toml";
static constexpr const char* DEFAULT_ITEMS_CONFIG_PATH = "common/config/items.toml";
static constexpr const char* DEFAULT_NPCS_CONFIG_PATH = "common/config/npcs.toml";

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << (argc > 0 ? argv[0] : "argentum_server") << " <port>\n";
        return EXIT_FAILURE;
    }

    const std::string port = argv[1];

    try {
        const char* game_config_env = "ARGENTUM_SERVER_CONFIG_FILE";
        const char* game_config_path_from_env = std::getenv(game_config_env);

        std::string game_config_filepath;
        std::string items_config_filepath;
        std::string map_config_filepath;
        std::string npcs_config_filepath;

        if (game_config_path_from_env) {
            // Entorno de producción (instalado): derivamos las rutas desde la variable de entorno.
            game_config_filepath = game_config_path_from_env;
            std::string config_dir = game_config_filepath;
            size_t last_slash = config_dir.find_last_of('/');
            if (last_slash != std::string::npos) {
                config_dir.resize(last_slash);
            } else {
                config_dir = ".";
            }
            items_config_filepath = config_dir + "/items.toml";
            map_config_filepath = config_dir + "/map.toml";
            npcs_config_filepath = config_dir + "/npcs.toml";
        } else {
            game_config_filepath = DEFAULT_GAME_CONFIG_PATH;
            items_config_filepath = DEFAULT_ITEMS_CONFIG_PATH;
            map_config_filepath = DEFAULT_MAP_CONFIG_PATH;
            npcs_config_filepath = DEFAULT_NPCS_CONFIG_PATH;
        }

        GameConfigLoader::load(game_config_filepath);
        ItemRegistryLoader::load(items_config_filepath);
        NPCRegistryLoader::load(npcs_config_filepath);
        auto world_factory = [&map_config_filepath]() {
            auto w = std::make_unique<World>(WorldMapLoader::load(map_config_filepath));
            WorldItemLoader::load_ground_items(*w, map_config_filepath);
            return w;
        };
        Server server(port, world_factory);
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
