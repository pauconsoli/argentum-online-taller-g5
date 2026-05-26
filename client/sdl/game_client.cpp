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
    renderer->load_texture("../client/assets/body.png");
}

GameClient::~GameClient() {
    delete renderer;
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void GameClient::run() {
    bool running = true;
    SDL_Event event;

    int frame_w = 51;
    int frame_h = 64;
    int frame_x = 0;
    int frame_y = 0;

    while (running) {
        while (SDL_PollEvent(&event)) {
            running = input_handler.handle(event);
        }

        renderer->clear();
        renderer->draw_frame(frame_x, frame_y, frame_w, frame_h, 400, 300);
        renderer->present();
    }
}