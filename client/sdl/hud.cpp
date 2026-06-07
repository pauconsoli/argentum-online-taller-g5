#include "hud.h"

#include <stdexcept>

Hud::Hud(SDL_Renderer* renderer, const std::string& font_path, int win_height):
    sdl_renderer(renderer), font(nullptr), window_height(win_height) {
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
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (surface == nullptr)
        return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(sdl_renderer, surface);
    SDL_FreeSurface(surface);
    if (texture == nullptr)
        return;

    int w, h;
    SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
    SDL_Rect dst = {x, y, w, h};
    SDL_RenderCopy(sdl_renderer, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
}

void Hud::draw_bar(int x, int y, int w, int h, int current, int max_val, SDL_Color color) {
    SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(sdl_renderer, 45, 45, 45, 210);
    SDL_Rect bg = {x, y, w, h};
    SDL_RenderFillRect(sdl_renderer, &bg);

    if (max_val > 0 && current > 0) {
        int filled = w * current / max_val;
        SDL_SetRenderDrawColor(sdl_renderer, color.r, color.g, color.b, color.a);
        SDL_Rect fill = {x, y, filled, h};
        SDL_RenderFillRect(sdl_renderer, &fill);
    }

    std::string label = std::to_string(current) + "/" + std::to_string(max_val);
    int tw = 0, th = 0;
    TTF_SizeText(font, label.c_str(), &tw, &th);
    int text_x = x + (w - tw) / 2;
    int text_y = y + (h - th) / 2;
    SDL_Color white = {255, 255, 255, 255};
    draw_text(label, text_x, text_y, white);
}

void Hud::draw(int hp, int max_hp, int mana, int max_mana, int level) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color red = {220, 55, 55, 255};
    SDL_Color blue = {55, 100, 255, 255};

    constexpr int panel_w = 155;
    constexpr int panel_h = 90;
    constexpr int margin = 5;
    int panel_x = margin;
    int panel_y = window_height - panel_h - margin;

    SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 150);
    SDL_Rect panel = {panel_x, panel_y, panel_w, panel_h};
    SDL_RenderFillRect(sdl_renderer, &panel);

    draw_text("Nivel: " + std::to_string(level), panel_x + 5, panel_y + 8, white);

    draw_bar(panel_x + 5, panel_y + 30, panel_w - 10, 20, hp, max_hp, red);

    draw_bar(panel_x + 5, panel_y + 57, panel_w - 10, 20, mana, max_mana, blue);
}
