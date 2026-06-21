#include <iostream>
#include <stdexcept>
#include <string>

#include "sdl/game_client.h"
#include "sdl/sdl_config.h"

int main(int argc, const char* const argv[]) {
    if (argc < 3) {
        std::cerr << "Uso: " << argv[0]
                  << " <host> <port> [--width W] [--height H] [--fullscreen]\n";
        return 1;
    }
    const std::string host = argv[1];
    const std::string port = argv[2];

    SdlConfig config = SdlConfig::load("client/config/sdl_config.toml");

    for (int i = 3; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--fullscreen") {
            config.fullscreen = true;
        } else if (arg == "--width" && i + 1 < argc) {
            config.window_width = std::stoi(argv[++i]);
        } else if (arg == "--height" && i + 1 < argc) {
            config.window_height = std::stoi(argv[++i]);
        }
    }

    try {
        GameClient game(config.window_width, config.window_height, config.fullscreen, host, port);
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
