#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <string>

class Renderer {
private:
    SDL_Renderer* sdl_renderer;

public:
    explicit Renderer(SDL_Window* window);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void draw_frame(SDL_Texture* texture, int frame_x, int frame_y, 
                    int frame_w, int frame_h, int x, int y);
    void clear();
    void present();
    SDL_Renderer* get_sdl_renderer() const;
};

#endif