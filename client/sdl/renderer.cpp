#include "renderer.h"
#include <stdexcept>

Renderer::Renderer(SDL_Window* window) {
    sdl_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (sdl_renderer == nullptr) {
        throw std::runtime_error(SDL_GetError());
    }
}

Renderer::~Renderer() {
    SDL_DestroyRenderer(sdl_renderer);
}

void Renderer::clear() {
    SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);
    SDL_RenderClear(sdl_renderer);
}

void Renderer::present() {
    SDL_RenderPresent(sdl_renderer);
}