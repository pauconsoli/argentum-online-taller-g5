#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

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
    SDL_Window* window;
    Renderer* renderer;
    Hud* hud;
    SpriteManager* sprite_manager;
    Client client;
    InputHandler input_handler;
    Camera camera;
    int player_x;
    int player_y;
    int width;
    int height;

 public:
    GameClient(int width, int height, const std::string& host, const std::string& port);
    ~GameClient();

    GameClient(const GameClient&) = delete;
    GameClient& operator=(const GameClient&) = delete;

    void run();
};

#endif
