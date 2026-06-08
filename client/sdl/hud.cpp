#include "hud.h"

#include <cstdint>
#include <stdexcept>
#include <vector>

Hud::Hud(SDL_Renderer* renderer, const std::string& font_path, int win_height, int win_width):
    sdl_renderer(renderer), font(nullptr), window_height(win_height), window_width(win_width) {
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

void Hud::draw_inventory(SpriteManager* sprites) {
    constexpr int COLS = 5;
    constexpr int ROWS = 4;
    constexpr int CELL = 40;
    constexpr int GAP = 2;
    constexpr int PAD = 8;
    constexpr int TITLE_H = 24;
    constexpr int MARGIN = 5;
    constexpr int ICON = 32;

    // TODO(cdelaurentis): reemplazar con el vector real del InventoryUpdate
    static const std::vector<uint16_t> items = {2, 30, 38, 37, 479, 132, 243, 1797};

    const int grid_w = COLS * CELL + (COLS - 1) * GAP;
    const int grid_h = ROWS * CELL + (ROWS - 1) * GAP;
    const int panel_w = grid_w + 2 * PAD;
    const int panel_h = PAD + TITLE_H + grid_h + PAD;
    const int panel_x = window_width - panel_w - MARGIN;
    const int panel_y = MARGIN;

    SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 170);
    SDL_Rect panel = {panel_x, panel_y, panel_w, panel_h};
    SDL_RenderFillRect(sdl_renderer, &panel);

    SDL_Color white = {255, 255, 255, 255};
    draw_text("Inventario", panel_x + PAD, panel_y + PAD / 2, white);

    const int grid_x = panel_x + PAD;
    const int grid_y = panel_y + PAD + TITLE_H;

    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            int cell_x = grid_x + col * (CELL + GAP);
            int cell_y = grid_y + row * (CELL + GAP);

            SDL_SetRenderDrawColor(sdl_renderer, 55, 55, 55, 255);
            SDL_Rect fill = {cell_x, cell_y, CELL, CELL};
            SDL_RenderFillRect(sdl_renderer, &fill);

            SDL_SetRenderDrawColor(sdl_renderer, 110, 110, 110, 255);
            SDL_RenderDrawRect(sdl_renderer, &fill);

            int slot = row * COLS + col;
            if (sprites != nullptr && slot < static_cast<int>(items.size())) {
                SDL_Texture* icon = sprites->get_item(items[slot]);
                if (icon != nullptr) {
                    int ox = cell_x + (CELL - ICON) / 2;
                    int oy = cell_y + (CELL - ICON) / 2;
                    SDL_Rect dst = {ox, oy, ICON, ICON};
                    SDL_RenderCopy(sdl_renderer, icon, nullptr, &dst);
                }
            }
        }
    }
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
