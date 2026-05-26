#ifndef HUD_H
#define HUD_H

#include <SDL2/SDL.h>
#include <SDL_ttf.h>
#include <string>

class Hud {
private:
    SDL_Renderer* sdl_renderer;
    TTF_Font* font;

    void draw_text(const std::string& text, int x, int y, SDL_Color color);

public:
    explicit Hud(SDL_Renderer* renderer, const std::string& font_path);
    ~Hud();

    Hud(const Hud&) = delete;
    Hud& operator=(const Hud&) = delete;

    void draw(int hp, int max_hp, int mana, int max_mana, int level);
};

#endif