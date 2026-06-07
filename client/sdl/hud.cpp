#include "hud.h"

#include <stdexcept>

Hud::Hud(SDL_Renderer* renderer, const std::string& font_path):
    sdl_renderer(renderer), font(nullptr) {
    if (TTF_Init() < 0) {
        throw std::runtime_error(TTF_GetError());
    }
    font = TTF_OpenFont(font_path.c_str(), 14);  // pasar al TOML
    if (!font)
        throw std::runtime_error(TTF_GetError());
}

Hud::~Hud() {
    TTF_CloseFont(font);
    TTF_Quit();
}

void Hud::draw_text(const std::string& text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (!surface)
        return;
    // imagen temporal
    SDL_Texture* texture = SDL_CreateTextureFromSurface(sdl_renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture)
        return;
    int w, h;
    SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
    SDL_Rect dst = {x, y, w, h};
    SDL_RenderCopy(sdl_renderer, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
}

void Hud::draw(int hp, int max_hp, int mana, int max_mana, int level, int xp, int max_xp, int gold,
               int win_w, int win_h) {

    // Pasar al TOML
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color red = {220, 50, 50, 255};
    SDL_Color blue = {50, 80, 220, 255};
    SDL_Color yellow = {255, 210, 40, 255};
    SDL_Color grey = {60, 60, 60, 255};


    int x = 15;
    int y = win_h - 160;

    draw_text("HP: " + std::to_string(hp) + "/" + std::to_string(max_hp), x, y, white);
    y += 20;

    SDL_SetRenderDrawColor(sdl_renderer, grey.r, grey.g, grey.b, 255);
    SDL_Rect hp_fondo = {x, y, 180, 16};
    SDL_RenderFillRect(sdl_renderer, &hp_fondo);

    int hp_ancho = 0;
    if (max_hp > 0)
        hp_ancho = hp * 180 / max_hp;
    SDL_SetRenderDrawColor(sdl_renderer, red.r, red.g, red.b, 255);
    SDL_Rect hp_relleno = {x, y, hp_ancho, 16};
    SDL_RenderFillRect(sdl_renderer, &hp_relleno);
    y += 24;

    draw_text("Mana: " + std::to_string(mana) + "/" + std::to_string(max_mana), x, y, white);
    y += 20;

    SDL_SetRenderDrawColor(sdl_renderer, grey.r, grey.g, grey.b, 255);
    SDL_Rect mana_fondo = {x, y, 180, 16};
    SDL_RenderFillRect(sdl_renderer, &mana_fondo);

    int mana_ancho = 0;
    if (max_mana > 0)
        mana_ancho = mana * 180 / max_mana;
    SDL_SetRenderDrawColor(sdl_renderer, blue.r, blue.g, blue.b, 255);
    SDL_Rect mana_relleno = {x, y, mana_ancho, 16};
    SDL_RenderFillRect(sdl_renderer, &mana_relleno);
    y += 24;

    draw_text("Nivel: " + std::to_string(level), x, y, white);
    y += 20;
    draw_text("XP: " + std::to_string(xp) + "/" + std::to_string(max_xp), x, y, yellow);
    y += 20;
    draw_text("Oro: " + std::to_string(gold), x, y, yellow);


    int inv_x = win_w - 230;
    int inv_y = win_h / 2 - 100;

    draw_text("Inventario", inv_x, inv_y, white);
    inv_y += 24;

    for (int fila = 0; fila < 4; fila++) {
        for (int col = 0; col < 5; col++) {
            int sx = inv_x + col * 43;
            int sy = inv_y + fila * 43;

            // fondo de  casilla
            SDL_SetRenderDrawColor(sdl_renderer, 40, 40, 40, 255);
            SDL_Rect casilla = {sx, sy, 40, 40};
            SDL_RenderFillRect(sdl_renderer, &casilla);

            // borde de la casilla
            SDL_SetRenderDrawColor(sdl_renderer, 100, 100, 100, 255);
            SDL_RenderDrawRect(sdl_renderer, &casilla);
        }
    }


    draw_text("Bienvenido a Argentum Online", 15, 15, white);
}
