#ifndef TERRAIN_RENDERER_H
#define TERRAIN_RENDERER_H

#include "../assets/sprite_manager.h"
#include "../core/camera.h"
#include "../state/client_map.h"
#include "common/world/terrain_type.h"
#include "renderer.h"

class TerrainRenderer {
 public:
    TerrainRenderer(Renderer* renderer, SpriteManager* sprite_manager, const Camera& camera);
    ~TerrainRenderer();

    void draw(int start_col, int end_col, int start_row, int end_row, int tile_w, int tile_h,
              const ClientMap& client_map, int screen_w, int screen_h);

    void draw_tree(int col, int row, int tile_w, int tile_h);

 private:
    Renderer* renderer_;
    SpriteManager* sprite_manager_;
    const Camera& camera_;

    SDL_Texture* terrain_cache_ = nullptr;
    int cache_col_ = -9999;
    int cache_row_ = -9999;
    int cache_w_ = 0;
    int cache_h_ = 0;

    // Estado de los cuatro vecinos cardinales de una celda respecto de un terreno dado.
    struct Cardinals {
        bool n, s, e, w;
    };

    // Retorna qué vecinos cardinales (N/S/E/W) de (col,row) son del terreno `t`.
    static Cardinals cardinals_of(const ClientMap& map, int col, int row, TerrainType t);

    // ¿La textura cacheada quedó obsoleta? (no existe, cambió el tamaño de pantalla,
    // o la cámara cruzó un tile boundary). Si es false, basta con blitear el cache.
    bool cache_is_stale(int start_col, int start_row, int screen_w, int screen_h) const;

    // Re-crea la textura si hace falta y vuelve a pintar las dos pasadas de terreno en ella.
    void rebuild_cache(int start_col, int end_col, int start_row, int end_row, int tile_w,
                       int tile_h, const ClientMap& client_map, int screen_w, int screen_h);

    void draw_base_tiles(int start_col, int end_col, int start_row, int end_row, int tile_w,
                         int tile_h, const ClientMap& client_map);
    void draw_terrain_transitions(int start_col, int end_col, int start_row, int end_row,
                                  int tile_w, int tile_h, const ClientMap& client_map);

    void draw_base_tile(int col, int row, int tile_w, int tile_h, const ClientMap& client_map);
    void draw_overlays_for_tile(int col, int row, int tile_w, int tile_h,
                                const ClientMap& client_map);

    // Dibuja los overlays de transición entre dos terrenos usando dos PNGs rotados
    // (<nombre_base>_edge y <nombre_base>_corner). Falla silenciosamente si faltan los PNGs.
    void draw_transition(int col, int row, int tile_w, int tile_h, int tx, int ty,
                         const ClientMap& client_map, TerrainType sobre, TerrainType vecino,
                         const char* nombre_base);

    // Dibuja los overlays de costa AO para celdas de WATER con vecinos SAND.
    // Usa un bitmask N/S/E/O para elegir entre las 6 piezas disponibles.
    // Las combinaciones sin pieza no dibujan nada (sin crash, sin overlay sintético).
    void draw_ao_costa_overlay(int col, int row, int tile_w, int tile_h, int tx, int ty,
                               const ClientMap& client_map);

    // Dibuja los overlays de transición arena→pasto sobre tiles de SAND con vecinos GRASS.
    // Usa 2 piezas base + espejado SDL para cubrir los 4 bordes rectos.
    // Las esquinas (2+ cardinales con pasto) se dejan sin overlay por ahora.
    void draw_ao_pasto_overlay(int col, int row, int tile_w, int tile_h, int tx, int ty,
                               const ClientMap& client_map);

    // Retorna false si (col,row) está fuera del mapa o si el terreno no coincide.
    static bool has_terrain(const ClientMap& map, int col, int row, TerrainType expected);
};

#endif
