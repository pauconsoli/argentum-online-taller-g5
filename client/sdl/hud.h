#ifndef HUD_H
#define HUD_H

#include <string>

#include <SDL2/SDL.h>
#include <SDL_ttf.h>

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

    void draw(int hp, int max_hp, int mana, int max_mana, int level);
    void draw_inventory(SpriteManager* sprites);
};

#endif
