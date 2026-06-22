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
    // --- Constantes de sombra (ajustar a ojo) ---
    // Opacidad de la elipse: 0 = transparente, 255 = opaco.
    static constexpr Uint8 SHADOW_ALPHA = 70;
    // Achatamiento vertical: ry = rx * SHADOW_FLATTEN (0.3 = muy aplastada, 0.5 = más redonda).
    static constexpr float SHADOW_FLATTEN = 0.35f;
    // Fracción de tile_w usada como diámetro de sombra (NPCs y jugadores).
    static constexpr float SHADOW_WIDTH_RATIO = 0.65f;
    // Fracción del ancho del tile usada como diámetro de sombra para árboles.
    static constexpr float SHADOW_TREE_RATIO = 0.75f;
    // Para NPCs con sprite grande (golems, zombies): sombra >= dw * este factor.
    // Garantiza que criaturas de 64-128px no queden con sombra más chica que las de 32px.
    static constexpr float SHADOW_LARGE_NPC_RATIO = 0.4f;

    // --- Constantes de contorno (ajustar a ojo) ---
    // Píxeles de desplazamiento del outline negro. 1 = sutil, 2 = marcado (mismo costo: 8 draws).
    static constexpr int OUTLINE_THICKNESS = 1;
    // Opacidad del negro de contorno: 255 = sólido, menos = más sutil.
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

    // Como draw_frame_scaled pero dibuja primero el frame teñido de negro en las 8 direcciones
    // cardinales y diagonales, desplazado `grosor` píxeles, formando un contorno visible.
    // IMPORTANTE: restaura el color mod a blanco antes de dibujar el sprite real.
    void draw_frame_scaled_outlined(SDL_Texture* texture, int frame_x, int frame_y, int frame_w,
                                    int frame_h, int x, int y, int draw_w, int draw_h, int grosor);

    // Igual que draw_frame pero aplica rotación clockwise en grados (0, 90, 180, 270).
    // El centro de rotación es el centro del tile destino.
    void draw_frame_rotated(SDL_Texture* texture, int frame_x, int frame_y, int frame_w,
                            int frame_h, int x, int y, double angle_deg);

    // Igual que draw_frame pero espeja la imagen horizontal o verticalmente.
    void draw_frame_flipped(SDL_Texture* texture, int frame_x, int frame_y, int frame_w,
                            int frame_h, int x, int y, SDL_RendererFlip flip);

    void draw_label(const std::string& text, int center_x, int y, SDL_Color color);

    // Elipse semitransparente aplastada centrada en (center_x, foot_y - ry).
    // foot_y es la línea de pies (base del tile). width es el diámetro horizontal.
    void draw_shadow(int center_x, int foot_y, int width);

    void clear();
    void present();
    SDL_Renderer* get_sdl_renderer() const;
};

#endif
