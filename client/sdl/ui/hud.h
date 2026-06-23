#ifndef HUD_H
#define HUD_H

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL_ttf.h>

#include "../assets/sprite_manager.h"
#include "common/updates/inventory_update.h"

class Hud {
 private:
    SDL_Renderer* sdl_renderer;
    TTF_Font* font;
    int window_height;
    int window_width;

    struct TextCache {
        SDL_Texture* texture = nullptr;
        std::string last_text;
        SDL_Color last_color = {0, 0, 0, 0};
    };

    static constexpr int INV_SLOTS = 20;
    enum : int {
        TCACHE_NIVEL,
        TCACHE_HP_LABEL,
        TCACHE_HP_BAR,
        TCACHE_MP_LABEL,
        TCACHE_MP_BAR,
        TCACHE_ORO,
        TCACHE_EXP,
        TCACHE_INV_TITLE,
        TCACHE_MUSIC_BTN,
        TCACHE_INV_QTY_BASE,
        TCACHE_COUNT = TCACHE_INV_QTY_BASE + INV_SLOTS
    };
    std::array<TextCache, TCACHE_COUNT> text_caches_{};

    // Garantiza que el slot tenga una textura actualizada. Devuelve la textura (puede ser nullptr).
    SDL_Texture* ensure_cached(int slot, const std::string& text, SDL_Color color);
    void draw_cached_text(int slot, const std::string& text, int x, int y, SDL_Color color);
    void draw_bar(int x, int y, int w, int h, int current, int max_val, SDL_Color color,
                  int text_slot);

 public:
    Hud(SDL_Renderer* renderer, const std::string& font_path, int win_height, int win_width);
    ~Hud();

    Hud(const Hud&) = delete;
    Hud& operator=(const Hud&) = delete;

    void draw(int hp, int max_hp, int mana, int max_mana, int level, uint64_t gold, uint64_t xp);
    void draw_inventory(SpriteManager* sprites, const std::vector<InventorySlotData>& slots,
                        int selected_slot = -1);
    void draw_music_button(bool paused);
    int get_slot_at(int screen_x, int screen_y) const;
    SDL_Rect get_music_button_rect() const;
};

#endif
