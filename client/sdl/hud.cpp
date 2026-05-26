#include "hud.h"
#include <stdexcept>

Hud::Hud(SDL_Renderer* renderer, const std::string& font_path) : 
    sdl_renderer(renderer), font(nullptr) {
    if (TTF_Init() < 0) {
        throw std::runtime_error(TTF_GetError());
    }
    font = TTF_OpenFont(font_path.c_str(), 16);
    if (font == nullptr) {
        throw std::runtime_error(TTF_GetError());
    }
}

Hud::~Hud() {
    TTF_CloseFont(font);
    TTF_Quit();
}

void Hud::draw_text(const std::string& text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (surface == nullptr) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(sdl_renderer, surface);
    SDL_FreeSurface(surface);
    if (texture == nullptr) return;

    int w, h;
    SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
    SDL_Rect dst = {x, y, w, h};
    SDL_RenderCopy(sdl_renderer, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
}

void Hud::draw(int hp, int max_hp, int mana, int max_mana, int level) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color red   = {255, 50, 50, 255};
    SDL_Color blue  = {50, 50, 255, 255};

    draw_text("Nivel: " + std::to_string(level), 10, 10, white);
    draw_text("HP: " + std::to_string(hp) + "/" + std::to_string(max_hp), 10, 30, red);
    draw_text("Mana: " + std::to_string(mana) + "/" + std::to_string(max_mana), 10, 50, blue);
}