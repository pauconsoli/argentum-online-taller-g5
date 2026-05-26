#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

#include <SDL2/SDL.h>
#include "renderer.h"
#include "input_handler.h"

class GameClient {
private:
    SDL_Window* window;
    Renderer* renderer;
    InputHandler input_handler;

public:
    GameClient(int width, int height);
    ~GameClient();

    GameClient(const GameClient&) = delete;
    GameClient& operator=(const GameClient&) = delete;

    void run();
};

#endif