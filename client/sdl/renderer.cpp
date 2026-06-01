#include "renderer.h"
#include <stdexcept>

Renderer::Renderer(SDL_Window* window) {
    sdl_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (sdl_renderer == nullptr) {
        throw std::runtime_error(SDL_GetError());
    }
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        SDL_DestroyRenderer(sdl_renderer);
        throw std::runtime_error(IMG_GetError());
    }
}

Renderer::~Renderer() {
    IMG_Quit();
    SDL_DestroyRenderer(sdl_renderer);
}

void Renderer::draw_frame(SDL_Texture* texture, int frame_x, int frame_y,
                          int frame_w, int frame_h, int x, int y) {
    SDL_Rect src = {frame_x, frame_y, frame_w, frame_h};
    SDL_Rect dst = {x, y, frame_w, frame_h};
    SDL_RenderCopy(sdl_renderer, texture, &src, &dst);
}

void Renderer::clear() {
    SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);
    SDL_RenderClear(sdl_renderer);
}

void Renderer::present() {
    SDL_RenderPresent(sdl_renderer);
}

SDL_Renderer* Renderer::get_sdl_renderer() const {
    return sdl_renderer;
}
