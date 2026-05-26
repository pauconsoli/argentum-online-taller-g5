#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

#include <SDL2/SDL.h>

#include "camera.h"
#include "hud.h"
#include "input_handler.h"
#include "renderer.h"


class GameClient {
 private:
    SDL_Window* window;
    Renderer* renderer;
    Hud* hud;
    InputHandler input_handler;
    Camera camera;
    int player_x;
    int player_y;

 public:
    GameClient(int width, int height);
    ~GameClient();

    GameClient(const GameClient&) = delete;
    GameClient& operator=(const GameClient&) = delete;

    void run();
};

#endif
