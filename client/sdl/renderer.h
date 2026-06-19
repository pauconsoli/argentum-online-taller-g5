#ifndef RENDERER_H
#define RENDERER_H

#include <string>

#include <SDL2/SDL.h>
#include <SDL_image.h>

class Renderer {
 private:
    SDL_Renderer* sdl_renderer;

 public:
    explicit Renderer(SDL_Window* window);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void draw_frame(SDL_Texture* texture, int frame_x, int frame_y, int frame_w, int frame_h, int x,
                    int y);
    void draw_frame_scaled(SDL_Texture* texture, int frame_x, int frame_y, int frame_w, int frame_h,
                           int x, int y, int draw_w, int draw_h);
    // Igual que draw_frame pero aplica rotación clockwise en grados (0, 90, 180, 270).
    // El centro de rotación es el centro del tile destino.
    void draw_frame_rotated(SDL_Texture* texture, int frame_x, int frame_y, int frame_w,
                            int frame_h, int x, int y, double angle_deg);
    void clear();
    void present();
    SDL_Renderer* get_sdl_renderer() const;
};

#endif
