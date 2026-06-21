#include "hud.h"

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "renderer.h"

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
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
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

void Hud::draw_inventory(SpriteManager* sprites, const std::vector<InventorySlotData>& slots,
                         int selected_slot) {
    constexpr int COLS = 5;
    constexpr int ROWS = 4;
    constexpr int CELL = 40;
    constexpr int GAP = 2;
    constexpr int PAD = 8;
    constexpr int TITLE_H = 24;
    constexpr int MARGIN = 5;
    constexpr int ICON = 32;

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
            int slot = row * COLS + col;

            SDL_SetRenderDrawColor(sdl_renderer, 55, 55, 55, 255);
            SDL_Rect fill = {cell_x, cell_y, CELL, CELL};
            SDL_RenderFillRect(sdl_renderer, &fill);

            // cppcheck-suppress syntaxError
            const int n_slots = static_cast<int>(slots.size());
            bool occupied = (slot < n_slots) && !slots[slot].item_name.empty();
            bool equipped = occupied && slots[slot].is_equipped;

            // Borde amarillo si equipado, gris si no
            if (equipped)
                SDL_SetRenderDrawColor(sdl_renderer, 255, 220, 0, 255);
            else
                SDL_SetRenderDrawColor(sdl_renderer, 110, 110, 110, 255);
            SDL_RenderDrawRect(sdl_renderer, &fill);

            // Borde celeste extra si es el slot seleccionado con click derecho
            if (slot == selected_slot) {
                SDL_Rect sel = {cell_x - 1, cell_y - 1, CELL + 2, CELL + 2};
                SDL_SetRenderDrawColor(sdl_renderer, 0, 200, 255, 255);
                SDL_RenderDrawRect(sdl_renderer, &sel);
            }

            if (sprites != nullptr && occupied) {
                std::string item_key = SpriteManager::item_key_for_name(slots[slot].item_name);
                SDL_Texture* icon = sprites->get_item(item_key);
                if (icon != nullptr) {
                    int ox = cell_x + (CELL - ICON) / 2;
                    int oy = cell_y + (CELL - ICON) / 2;
                    SDL_Rect dst = {ox, oy, ICON, ICON};
                    // Contorno negro fino: misma técnica que Renderer::draw_frame_scaled_outlined
                    static const int dirs[8][2] = {{-1, 0},  {1, 0},  {0, -1}, {0, 1},
                                                   {-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
                    SDL_SetTextureColorMod(icon, 0, 0, 0);
                    SDL_SetTextureAlphaMod(icon, Renderer::OUTLINE_ALPHA);
                    for (const auto& d : dirs) {
                        SDL_Rect off = {ox + d[0] * Renderer::OUTLINE_THICKNESS,
                                        oy + d[1] * Renderer::OUTLINE_THICKNESS, ICON, ICON};
                        SDL_RenderCopy(sdl_renderer, icon, nullptr, &off);
                    }
                    SDL_SetTextureAlphaMod(icon, 255);
                    SDL_SetTextureColorMod(icon, 255, 255, 255);
                    SDL_RenderCopy(sdl_renderer, icon, nullptr, &dst);
                }
                uint32_t qty = slots[slot].quantity;
                if (qty > 1) {
                    std::string qty_str = std::to_string(qty);
                    int tw = 0, th = 0;
                    TTF_SizeUTF8(font, qty_str.c_str(), &tw, &th);
                    SDL_Color yellow = {255, 220, 0, 255};
                    draw_text(qty_str, cell_x + CELL - tw - 2, cell_y + CELL - th, yellow);
                }
            }
        }
    }
}

int Hud::get_slot_at(int screen_x, int screen_y) const {
    constexpr int COLS = 5;
    constexpr int ROWS = 4;
    constexpr int CELL = 40;
    constexpr int GAP = 2;
    constexpr int PAD = 8;
    constexpr int TITLE_H = 24;
    constexpr int MARGIN = 5;

    const int grid_w = COLS * CELL + (COLS - 1) * GAP;
    const int panel_w = grid_w + 2 * PAD;
    const int panel_x = window_width - panel_w - MARGIN;
    const int panel_y = MARGIN;

    const int grid_x = panel_x + PAD;
    const int grid_y = panel_y + PAD + TITLE_H;

    int rel_x = screen_x - grid_x;
    int rel_y = screen_y - grid_y;

    if (rel_x < 0 || rel_y < 0)
        return -1;

    int col = rel_x / (CELL + GAP);
    int row = rel_y / (CELL + GAP);

    if (col >= COLS || row >= ROWS)
        return -1;

    if (rel_x % (CELL + GAP) >= CELL || rel_y % (CELL + GAP) >= CELL)
        return -1;

    return row * COLS + col;
}

void Hud::draw_music_button(bool paused) {
    constexpr int BTN_W = 140;
    constexpr int BTN_H = 24;
    constexpr int MARGIN = 5;
    const int btn_x = window_width - BTN_W - MARGIN;
    const int btn_y = window_height - BTN_H - MARGIN;

    SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 160);
    SDL_Rect bg = {btn_x, btn_y, BTN_W, BTN_H};
    SDL_RenderFillRect(sdl_renderer, &bg);

    SDL_Color color = paused ? SDL_Color{180, 60, 60, 255} : SDL_Color{60, 200, 100, 255};
    std::string label = paused ? "[M] Musica: OFF" : "[M] Musica: ON";
    draw_text(label, btn_x + 5, btn_y + 4, color);
}

SDL_Rect Hud::get_music_button_rect() const {
    constexpr int BTN_W = 140;
    constexpr int BTN_H = 24;
    constexpr int MARGIN = 5;
    return {window_width - BTN_W - MARGIN, window_height - BTN_H - MARGIN, BTN_W, BTN_H};
}

void Hud::draw(int hp, int max_hp, int mana, int max_mana, int level, uint64_t gold, uint64_t xp) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color yellow = {255, 220, 60, 255};
    SDL_Color gray = {180, 180, 180, 255};
    SDL_Color red = {220, 55, 55, 255};
    SDL_Color blue = {55, 100, 255, 255};

    constexpr int LABEL_W = 30;  // ancho reservado para "HP " / "MP "
    constexpr int BAR_H = 18;
    constexpr int ROW_GAP = 4;  // espacio entre filas
    constexpr int PAD = 6;
    constexpr int margin = 5;

    // filas: Nivel, HP, MP, Oro, Exp  → altura total
    constexpr int panel_w = 190;
    constexpr int panel_h = PAD + 20 + ROW_GAP  // Nivel
                            + BAR_H + ROW_GAP   // HP
                            + BAR_H + ROW_GAP   // MP
                            + 18 + ROW_GAP      // Oro
                            + 18 + PAD;         // Exp
    int panel_x = margin;
    int panel_y = window_height - panel_h - margin;

    SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 160);
    SDL_Rect panel = {panel_x, panel_y, panel_w, panel_h};
    SDL_RenderFillRect(sdl_renderer, &panel);

    int cy = panel_y + PAD;

    // — Nivel —
    draw_text("Nivel: " + std::to_string(level), panel_x + PAD, cy, white);
    cy += 20 + ROW_GAP;

    // — Vida —
    draw_text("HP", panel_x + PAD, cy + 1, red);
    draw_bar(panel_x + PAD + LABEL_W, cy, panel_w - PAD * 2 - LABEL_W, BAR_H, hp, max_hp, red);
    cy += BAR_H + ROW_GAP;

    // — Maná —
    draw_text("MP", panel_x + PAD, cy + 1, blue);
    draw_bar(panel_x + PAD + LABEL_W, cy, panel_w - PAD * 2 - LABEL_W, BAR_H, mana, max_mana, blue);
    cy += BAR_H + ROW_GAP;

    // — Oro —
    draw_text("Oro:  " + std::to_string(gold), panel_x + PAD, cy, yellow);
    cy += 18 + ROW_GAP;

    // — Experiencia —
    draw_text("Exp:  " + std::to_string(xp), panel_x + PAD, cy, gray);
}
