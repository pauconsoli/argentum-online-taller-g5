#include "mini_chat.h"

#include <stdexcept>

MiniChat::MiniChat(SDL_Renderer* renderer, const std::string& font_path, int win_width):
    sdl_renderer(renderer), font(nullptr), window_width(win_width) {
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

void MiniChat::draw_label(const std::string& text, int center_x, int y, SDL_Color color) {
    if (text.empty())
        return;
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface)
        return;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(sdl_renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture)
        return;
    int w, h;
    SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
    SDL_Rect dst = {center_x - w / 2, y, w, h};
    SDL_RenderCopy(sdl_renderer, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
}

void MiniChat::add_message(const std::string& msg) {
    messages.push_back(msg);
    while (static_cast<int>(messages.size()) > MAX_MESSAGES) {
        messages.pop_front();
    }
}

void MiniChat::draw_input(const std::string& input, int y) {
    const std::string display = "> " + input + "|";
    int box_w = window_width * 6 / 10;
    SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 200);
    SDL_Rect box = {0, y, box_w, LINE_HEIGHT + 2 * PADDING};
    SDL_RenderFillRect(sdl_renderer, &box);
    SDL_Color white = {255, 255, 255, 255};
    draw_text(display, 10, y + PADDING, white);
}

void MiniChat::draw() {
    if (messages.empty())
        return;

    SDL_Color color = {255, 220, 100, 255};
    int strip_w = window_width * 6 / 10;
    int strip_h = static_cast<int>(messages.size()) * LINE_HEIGHT + 2 * PADDING;

    SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 180);
    SDL_Rect strip = {0, 0, strip_w, strip_h};
    SDL_RenderFillRect(sdl_renderer, &strip);

    int y = PADDING;
    for (const auto& msg : messages) {
        draw_text(msg, 10, y, color);
        y += LINE_HEIGHT;
    }
}
