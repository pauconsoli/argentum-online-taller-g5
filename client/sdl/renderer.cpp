#include "renderer.h"

#include <stdexcept>

// no decide que dibujar, solo sabe dibujar

Renderer::Renderer(SDL_Window* window) {
    // se crea el renderer asociado a la ventana, con aceleración por hardware
    sdl_renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED);  // pide usar gpu, para no gastar cpu
    if (sdl_renderer == nullptr) {
        throw std::runtime_error(SDL_GetError());
    }
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        SDL_DestroyRenderer(sdl_renderer);
        throw std::runtime_error(IMG_GetError());
    }
}

Renderer::~Renderer() {
    IMG_Quit();
    SDL_DestroyRenderer(sdl_renderer);
}

void Renderer::draw_frame(SDL_Texture* texture, int frame_x, int frame_y, int frame_w, int frame_h,
                          int x, int y) {
    SDL_Rect src = {frame_x, frame_y, frame_w,
                    frame_h};  // src dice qué parte de la imagen original querés recortar.
    SDL_Rect dst = {x, y, frame_w, frame_h};  // dst dice dónde dibujar ese frame en la pantalla.
    SDL_RenderCopy(sdl_renderer, texture, &src, &dst);
}

void Renderer::clear() {
    // Pinta toda la pantalla de negro y borra lo anterior. Se llama al inicio de cada frame.
    SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);
    SDL_RenderClear(sdl_renderer);
}

void Renderer::present() {
    SDL_RenderPresent(sdl_renderer);
}

SDL_Renderer* Renderer::get_sdl_renderer() const {
    return sdl_renderer;
}

// El flujo típico es:

// renderer->clear();

// renderer->draw_frame(...); // pasto
// renderer->draw_frame(...); // cuerpo
// renderer->draw_frame(...); // cabeza
// hud->draw(...);

// renderer->present();
