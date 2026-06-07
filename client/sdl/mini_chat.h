#ifndef MINI_CHAT_H
#define MINI_CHAT_H

#include <deque>
#include <string>

#include <SDL2/SDL.h>
#include <SDL_ttf.h>

class MiniChat {
 private:
    static constexpr int MAX_MESSAGES = 8;
    SDL_Renderer* sdl_renderer;
    TTF_Font* font;
    std::deque<std::string> messages;

    void draw_text(const std::string& text, int x, int y, SDL_Color color);

 public:
    MiniChat(SDL_Renderer* renderer, const std::string& font_path);
    ~MiniChat();

    MiniChat(const MiniChat&) = delete;
    MiniChat& operator=(const MiniChat&) = delete;

    void add_message(const std::string& msg);
    void draw();
};

#endif
