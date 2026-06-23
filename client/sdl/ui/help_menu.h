#ifndef HELP_MENU_H
#define HELP_MENU_H

#include <string>
#include <utility>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL_ttf.h>

class HelpMenu {
 private:
    SDL_Renderer* sdl_renderer;
    TTF_Font* font;
    int window_width;
    int window_height;
    bool visible;
    // Cada par es {comando, descripcion}. Si la descripcion esta vacia, es un titulo.
    std::vector<std::pair<std::string, std::string>> left;
    std::vector<std::pair<std::string, std::string>> right;
    std::vector<std::pair<std::string, std::string>> cheats;

    void draw_text(const std::string& text, int x, int y, SDL_Color color);

 public:
    HelpMenu(SDL_Renderer* renderer, const std::string& font_path, int win_width, int win_height);
    ~HelpMenu();

    HelpMenu(const HelpMenu&) = delete;
    HelpMenu& operator=(const HelpMenu&) = delete;

    void toggle();
    void draw();
};

#endif
