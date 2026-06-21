#ifndef HUD_H
#define HUD_H

#include <cstdint>
#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL_ttf.h>

#include "common/updates/inventory_update.h"
#include "sprite_manager.h"

class Hud {
 private:
    SDL_Renderer* sdl_renderer;
    TTF_Font* font;
    int window_height;
    int window_width;

    void draw_text(const std::string& text, int x, int y, SDL_Color color);
    void draw_bar(int x, int y, int w, int h, int current, int max_val, SDL_Color color);

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
