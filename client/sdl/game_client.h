#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

#include <cstdint>
#include <string>

#include <SDL2/SDL.h>

#include "../client.h"
#include "camera.h"
#include "hud.h"
#include "input_handler.h"
#include "renderer.h"
#include "sprite_manager.h"

class GameClient {
 private:
    // punteros, deben estar en el destructor
    SDL_Window* window;
    Renderer* renderer;
    Hud* hud;
    SpriteManager* sprite_manager;
    // conector con el servidor
    Client client;
    InputHandler input_handler;
    Camera camera;

    // Guarda el id del jugador propio y su posición. Esto es limitado, TODO
    uint32_t my_player_id;
    int player_x;
    int player_y;

    // Guarda tamaño de ventana.
    int width;
    int height;

 public:
    // con TOML game client va a dejar de recibir los datos harcodeados
    GameClient(int width, int height, const std::string& host, const std::string& port);
    ~GameClient();

    GameClient(const GameClient&) = delete;
    GameClient& operator=(const GameClient&) = delete;

    void run();
};

#endif
