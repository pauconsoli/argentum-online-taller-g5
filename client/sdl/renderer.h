#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>

class Renderer {
private:
    SDL_Renderer* sdl_renderer;

public:
    explicit Renderer(SDL_Window* window);
    ~Renderer();

    // No permitimos copiar el Renderer
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void clear();
    void present();
};

#endif