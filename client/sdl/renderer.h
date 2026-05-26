#ifndef RENDERER_H
#define RENDERER_H

#include <string>

#include <SDL2/SDL.h>
#include <SDL_image.h>

class Renderer {
 private:
    SDL_Renderer* sdl_renderer;
    SDL_Texture* texture;

 public:
    explicit Renderer(SDL_Window* window);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void load_texture(const std::string& path);
    void draw_frame(int frame_x, int frame_y, int frame_w, int frame_h, int x, int y);
    void clear();
    void present();
    SDL_Renderer* get_sdl_renderer() const;
};

#endif
