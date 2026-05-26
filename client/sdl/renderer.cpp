#include "renderer.h"
#include <stdexcept>

Renderer::Renderer(SDL_Window* window) : texture(nullptr) {
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
    if (texture != nullptr) {
        SDL_DestroyTexture(texture);
    }
    IMG_Quit();
    SDL_DestroyRenderer(sdl_renderer);
}

void Renderer::load_texture(const std::string& path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (surface == nullptr) {
        throw std::runtime_error(IMG_GetError());
    }
    texture = SDL_CreateTextureFromSurface(sdl_renderer, surface);
    SDL_FreeSurface(surface);
    if (texture == nullptr) {
        throw std::runtime_error(SDL_GetError());
    }
}

void Renderer::draw_frame(int frame_x, int frame_y, int frame_w, int frame_h, int x, int y) {
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