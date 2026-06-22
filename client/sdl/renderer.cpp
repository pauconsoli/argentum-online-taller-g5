#include "renderer.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>

Renderer::Renderer(SDL_Window* window) {
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    sdl_renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    if (sdl_renderer == nullptr) {
        throw std::runtime_error(SDL_GetError());
    }
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        SDL_DestroyRenderer(sdl_renderer);
        throw std::runtime_error(IMG_GetError());
    }
    TTF_Init();
}

Renderer::~Renderer() {
    for (auto& [key, tex] : label_cache) SDL_DestroyTexture(tex);
    if (font)
        TTF_CloseFont(font);
    TTF_Quit();
    IMG_Quit();
    SDL_DestroyRenderer(sdl_renderer);
}

void Renderer::load_font(const std::string& path, int size) {
    if (font)
        TTF_CloseFont(font);
    font = TTF_OpenFont(path.c_str(), size);
}

void Renderer::draw_frame(SDL_Texture* texture, int frame_x, int frame_y, int frame_w, int frame_h,
                          int x, int y) {
    SDL_Rect src = {frame_x, frame_y, frame_w, frame_h};
    SDL_Rect dst = {x, y, frame_w, frame_h};
    SDL_RenderCopy(sdl_renderer, texture, &src, &dst);
}

void Renderer::draw_frame_scaled(SDL_Texture* texture, int frame_x, int frame_y, int frame_w,
                                 int frame_h, int x, int y, int draw_w, int draw_h) {
    SDL_Rect src = {frame_x, frame_y, frame_w, frame_h};
    SDL_Rect dst = {x, y, draw_w, draw_h};
    SDL_RenderCopy(sdl_renderer, texture, &src, &dst);
}

void Renderer::draw_frame_scaled_outlined(SDL_Texture* texture, int frame_x, int frame_y,
                                          int frame_w, int frame_h, int x, int y, int draw_w,
                                          int draw_h, int grosor) {
    // 4 cardinales (5 draws/sprite). Para volver a 8+1, descomentar las diagonales y cambiar [4] →
    // [8].
    static const int dirs[4][2] = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1},  // N, S, E, O
        // {-1, -1}, {1, -1}, {-1, 1}, {1, 1},
    };

    // Teñir de negro semitransparente para las copias del contorno
    SDL_SetTextureColorMod(texture, 0, 0, 0);
    SDL_SetTextureAlphaMod(texture, OUTLINE_ALPHA);
    for (const int* d : dirs) {
        draw_frame_scaled(texture, frame_x, frame_y, frame_w, frame_h, x + d[0] * grosor,
                          y + d[1] * grosor, draw_w, draw_h);
    }

    // Restaurar color mod y alpha ANTES de dibujar el sprite real
    SDL_SetTextureAlphaMod(texture, 255);
    SDL_SetTextureColorMod(texture, 255, 255, 255);
    draw_frame_scaled(texture, frame_x, frame_y, frame_w, frame_h, x, y, draw_w, draw_h);
}

void Renderer::draw_frame_rotated(SDL_Texture* texture, int frame_x, int frame_y, int frame_w,
                                  int frame_h, int x, int y, double angle_deg) {
    SDL_Rect src = {frame_x, frame_y, frame_w, frame_h};
    SDL_Rect dst = {x, y, frame_w, frame_h};
    SDL_RenderCopyEx(sdl_renderer, texture, &src, &dst, angle_deg, nullptr, SDL_FLIP_NONE);
}

void Renderer::draw_frame_flipped(SDL_Texture* texture, int frame_x, int frame_y, int frame_w,
                                  int frame_h, int x, int y, SDL_RendererFlip flip) {
    SDL_Rect src = {frame_x, frame_y, frame_w, frame_h};
    SDL_Rect dst = {x, y, frame_w, frame_h};
    SDL_RenderCopyEx(sdl_renderer, texture, &src, &dst, 0.0, nullptr, flip);
}

void Renderer::clear() {
    SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);
    SDL_RenderClear(sdl_renderer);
}

void Renderer::present() {
    SDL_RenderPresent(sdl_renderer);
}

SDL_Renderer* Renderer::get_sdl_renderer() const {
    return sdl_renderer;
}

void Renderer::draw_shadow(int center_x, int foot_y, int width) {
    int rx = width / 2;
    if (rx <= 0)
        return;
    int ry = std::max(1, static_cast<int>(rx * SHADOW_FLATTEN));
    int cy = foot_y - ry;  // la elipse toca foot_y en su borde inferior

    SDL_BlendMode prev_blend;
    SDL_GetRenderDrawBlendMode(sdl_renderer, &prev_blend);
    SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, SHADOW_ALPHA);

    for (int dy = -ry; dy <= ry; dy++) {
        float t = static_cast<float>(dy) / static_cast<float>(ry);
        int dx = static_cast<int>(rx * sqrtf(1.0f - t * t));
        SDL_RenderDrawLine(sdl_renderer, center_x - dx, cy + dy, center_x + dx, cy + dy);
    }

    SDL_SetRenderDrawBlendMode(sdl_renderer, prev_blend);
}

void Renderer::draw_label(const std::string& text, int center_x, int y, SDL_Color color) {
    if (text.empty())
        return;

    char key_buf[512];
    std::snprintf(key_buf, sizeof(key_buf), "%s|%u,%u,%u,%u", text.c_str(), color.r, color.g,
                  color.b, color.a);
    std::string key(key_buf);

    SDL_Texture* texture = nullptr;
    auto it = label_cache.find(key);
    if (it != label_cache.end()) {
        texture = it->second;
    } else {
        SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
        if (!surface)
            return;
        texture = SDL_CreateTextureFromSurface(sdl_renderer, surface);
        SDL_FreeSurface(surface);
        if (!texture)
            return;
        label_cache[key] = texture;
    }

    int w, h;
    SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
    SDL_Rect dst = {center_x - w / 2, y, w, h};
    SDL_RenderCopy(sdl_renderer, texture, nullptr, &dst);
}
