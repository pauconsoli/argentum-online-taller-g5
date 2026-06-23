#ifndef HUD_H
#define HUD_H

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL_ttf.h>

#include "../assets/sprite_manager.h"
#include "common/updates/inventory_update.h"

class Hud {
 private:
    SDL_Renderer* sdl_renderer;  // renderer al que se dibuja (no owneado)
    TTF_Font* font;              // fuente de 16px para todo el texto del HUD
    int window_height;  // alto de la ventana, para posicionar el panel en el borde inferior
    int window_width;   // ancho de la ventana, para posicionar inventario y boton de musica

    // cache de textura para un texto: evita re-renderizar strings que no cambiaron
    struct TextCache {
        SDL_Texture* texture = nullptr;  // textura SDL generada por TTF; nullptr si aun no existe
        std::string last_text;           // ultimo string renderizado, para detectar cambios
        SDL_Color last_color = {0, 0, 0, 0};  // ultimo color; si cambia, hay que re-renderizar
    };

    static constexpr int INV_SLOTS = 20;  // 5 columnas x 4 filas de inventario

    // indices del array de caches; cada uno corresponde a un elemento del HUD
    enum : int {
        TCACHE_NIVEL,         // texto "Nivel: N"
        TCACHE_HP_LABEL,      // etiqueta "HP"
        TCACHE_HP_BAR,        // texto "actual/max" centrado en la barra de vida
        TCACHE_MP_LABEL,      // etiqueta "MP"
        TCACHE_MP_BAR,        // texto "actual/max" centrado en la barra de mana
        TCACHE_ORO,           // texto "Oro: N"
        TCACHE_EXP,           // texto "Exp: N"
        TCACHE_INV_TITLE,     // titulo "Inventario"
        TCACHE_MUSIC_BTN,     // texto del boton "[M] Musica: ON/OFF"
        TCACHE_INV_QTY_BASE,  // primer indice de cantidad por slot de inventario
        TCACHE_COUNT = TCACHE_INV_QTY_BASE + INV_SLOTS  // total de slots del array
    };
    std::array<TextCache, TCACHE_COUNT> text_caches_{};  // array fijo, inicializado en {}

    // si el texto o color cambiaron, destruye la textura vieja y genera una nueva; devuelve la
    // textura
    SDL_Texture* ensure_cached(int slot, const std::string& text, SDL_Color color);

    // dibuja el texto del slot en (x, y); llama a ensure_cached internamente
    void draw_cached_text(int slot, const std::string& text, int x, int y, SDL_Color color);

    // dibuja una barra de progreso con fondo gris, relleno de `color` proporcional a
    // current/max_val, y el texto "current/max_val" centrado; usa text_slot como cache de ese texto
    void draw_bar(int x, int y, int w, int h, int current, int max_val, SDL_Color color,
                  int text_slot);

 public:
    // carga la fuente (SDL_ttf ya fue inicializado por GameClient)
    Hud(SDL_Renderer* renderer, const std::string& font_path, int win_height, int win_width);
    ~Hud();  // destruye todas las texturas cacheadas y cierra la fuente

    Hud(const Hud&) = delete;             // no se puede copiar (owna texturas SDL)
    Hud& operator=(const Hud&) = delete;  // idem

    // dibuja el panel inferior izquierdo con nivel, barras de HP/MP, oro y experiencia
    void draw(int hp, int max_hp, int mana, int max_mana, int level, uint64_t gold, uint64_t xp);

    // dibuja el panel de inventario (5x4) en la esquina superior derecha
    // `selected_slot` es el slot marcado con click derecho (-1 si ninguno)
    void draw_inventory(SpriteManager* sprites, const std::vector<InventorySlotData>& slots,
                        int selected_slot = -1);

    // dibuja el boton de musica en la esquina inferior derecha; color rojo si pausado, verde si
    // activo
    void draw_music_button(bool paused);

    // dado un punto de pantalla (screen_x, screen_y), devuelve el indice de slot (0-19)
    // o -1 si el punto no cae dentro de ningun slot del inventario
    int get_slot_at(int screen_x, int screen_y) const;

    // devuelve el SDL_Rect del boton de musica (para detectar clicks en el input handler)
    SDL_Rect get_music_button_rect() const;
};

#endif
