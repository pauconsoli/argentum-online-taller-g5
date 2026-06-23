#ifndef RENDERER_H
#define RENDERER_H

#include <string>
#include <unordered_map>

#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

class Renderer {
 private:
    SDL_Renderer* sdl_renderer;
    TTF_Font* font = nullptr;
    std::unordered_map<std::string, SDL_Texture*> label_cache;

 public:
    static constexpr Uint8 SHADOW_ALPHA = 70;
    static constexpr float SHADOW_FLATTEN = 0.35f;
    static constexpr float SHADOW_WIDTH_RATIO = 0.65f;
    static constexpr float SHADOW_TREE_RATIO = 0.75f;
    static constexpr float SHADOW_LARGE_NPC_RATIO = 0.4f;

    static constexpr int OUTLINE_THICKNESS = 1;
    static constexpr Uint8 OUTLINE_ALPHA = 140;

    explicit Renderer(SDL_Window* window);
    void load_font(const std::string& path, int size);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void draw_frame(SDL_Texture* texture, int frame_x, int frame_y, int frame_w, int frame_h, int x,
                    int y);
    void draw_frame_scaled(SDL_Texture* texture, int frame_x, int frame_y, int frame_w, int frame_h,
                           int x, int y, int draw_w, int draw_h);

    void draw_frame_scaled_outlined(SDL_Texture* texture, int frame_x, int frame_y, int frame_w,
                                    int frame_h, int x, int y, int draw_w, int draw_h, int grosor);

    void draw_frame_rotated(SDL_Texture* texture, int frame_x, int frame_y, int frame_w,
                            int frame_h, int x, int y, double angle_deg);

    void draw_frame_flipped(SDL_Texture* texture, int frame_x, int frame_y, int frame_w,
                            int frame_h, int x, int y, SDL_RendererFlip flip);

    void draw_label(const std::string& text, int center_x, int y, SDL_Color color);

    void draw_shadow(int center_x, int foot_y, int width);

    void clear();
    void present();
    SDL_Renderer* get_sdl_renderer() const;
};

#endif
