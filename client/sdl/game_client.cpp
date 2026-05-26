#include "game_client.h"
#include <stdexcept>

GameClient::GameClient(int width, int height) : window(nullptr), renderer(nullptr) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error(SDL_GetError());
    }

    window = SDL_CreateWindow(
        "Argentum Online - G5",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_SHOWN
    );
    if (window == nullptr) {
        SDL_Quit();
        throw std::runtime_error(SDL_GetError());
    }

    renderer = new Renderer(window);
}

GameClient::~GameClient() {
    delete renderer;
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void GameClient::run() {
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            running = input_handler.handle(event);
        }

        renderer->clear();
        renderer->present();
    }
}