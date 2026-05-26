#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

#include <SDL2/SDL.h>
#include "renderer.h"

class GameClient {
private:
    SDL_Window* window;
    Renderer* renderer;

public:
    GameClient(int width, int height);
    ~GameClient();

    GameClient(const GameClient&) = delete;
    GameClient& operator=(const GameClient&) = delete;

    void run();
};

#endif