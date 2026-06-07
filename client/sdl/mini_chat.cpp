#include "mini_chat.h"

#include <stdexcept>

MiniChat::MiniChat(SDL_Renderer* renderer, const std::string& font_path):
    sdl_renderer(renderer), font(nullptr) {
    if (TTF_Init() < 0) {
        throw std::runtime_error(TTF_GetError());
    }
    font = TTF_OpenFont(font_path.c_str(), 14);
    if (font == nullptr) {
        throw std::runtime_error(TTF_GetError());
    }
}

MiniChat::~MiniChat() {
    TTF_CloseFont(font);
    TTF_Quit();
}

void MiniChat::draw_text(const std::string& text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), color);
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

void MiniChat::add_message(const std::string& msg) {
    messages.push_back(msg);
    while (static_cast<int>(messages.size()) > MAX_MESSAGES) {
        messages.pop_front();
    }
}

void MiniChat::draw() {
    SDL_Color color = {255, 220, 100, 255};
    constexpr int x = 10;
    constexpr int line_height = 18;
    int y = 80;
    for (const auto& msg : messages) {
        draw_text(msg, x, y, color);
        y += line_height;
    }
}
