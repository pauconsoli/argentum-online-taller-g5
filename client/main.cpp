#include <iostream>
#include <stdexcept>
#include "sdl/game_client.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <host> <port>\n";
        return 1;
    }
    try {
        GameClient game(800, 600, argv[1], argv[2]);
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}