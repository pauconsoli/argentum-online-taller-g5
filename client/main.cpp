#include <iostream>
#include <stdexcept>

#include "sdl/game_client.h"

int main(int argc, const char* argv[]) {
    // le mando taller_client localhost 8080
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <host> <port>\n";
        return 1;
    }
    try {
        // TO DO: pasa alto y ancho de ventana -> se tiene que leer de TOML
        GameClient game(800, 600, argv[1], argv[2]);
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
