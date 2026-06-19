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
    TTF_Init();
}

Renderer::~Renderer() {
    if (font)
        TTF_CloseFont(font);
    TTF_Quit();
    IMG_Quit();
    SDL_DestroyRenderer(sdl_renderer);
}

void Renderer::load_font(const std::string& path, int size) {
    if (font)
        TTF_CloseFont(font);
    font = TTF_OpenFont(path.c_str(), size);
}

void Renderer::draw_frame(SDL_Texture* texture, int frame_x, int frame_y, int frame_w, int frame_h,
                          int x, int y) {
    SDL_Rect src = {frame_x, frame_y, frame_w, frame_h};
    SDL_Rect dst = {x, y, frame_w, frame_h};
    SDL_RenderCopy(sdl_renderer, texture, &src, &dst);
}

void Renderer::draw_frame_scaled(SDL_Texture* texture, int frame_x, int frame_y, int frame_w,
                                 int frame_h, int x, int y, int draw_w, int draw_h) {
    SDL_Rect src = {frame_x, frame_y, frame_w, frame_h};
    SDL_Rect dst = {x, y, draw_w, draw_h};
    SDL_RenderCopy(sdl_renderer, texture, &src, &dst);
}

void Renderer::draw_frame_rotated(SDL_Texture* texture, int frame_x, int frame_y, int frame_w,
                                  int frame_h, int x, int y, double angle_deg) {
    SDL_Rect src = {frame_x, frame_y, frame_w, frame_h};
    SDL_Rect dst = {x, y, frame_w, frame_h};
    SDL_RenderCopyEx(sdl_renderer, texture, &src, &dst, angle_deg, nullptr, SDL_FLIP_NONE);
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

void Renderer::draw_label(const std::string& text, int center_x, int y, SDL_Color color) {
    if (text.empty())
        return;
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface)
        return;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(sdl_renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture)
        return;
    int w, h;
    SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
    SDL_Rect dst = {center_x - w / 2, y, w, h};
    SDL_RenderCopy(sdl_renderer, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
}
