#include <iostream>
#include <stdexcept>
#include "sdl/game_client.h"

int main() {
    try {
        GameClient game(800, 600);
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}