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

    struct Cardinals {
        bool north, south, east, west;
    };

    static Cardinals cardinals_of(const ClientMap& map, int col, int row, TerrainType t);

    bool cache_is_old(int start_col, int start_row, int screen_w, int screen_h) const;
    void rebuild_cache(int start_col, int end_col, int start_row, int end_row, int tile_w,
                       int tile_h, const ClientMap& client_map, int screen_w, int screen_h);

    void draw_base_tiles(int start_col, int end_col, int start_row, int end_row, int tile_w,
                         int tile_h, const ClientMap& client_map);
    void draw_terrain_transitions(int start_col, int end_col, int start_row, int end_row,
                                  int tile_w, int tile_h, const ClientMap& client_map);

    void draw_base_tile(int col, int row, int tile_w, int tile_h, const ClientMap& client_map);
    void draw_overlays_for_tile(int col, int row, int tile_w, int tile_h,
                                const ClientMap& client_map);


    void draw_transition(int col, int row, int tile_w, int tile_h, int tx, int ty,
                         const ClientMap& client_map, TerrainType sobre, TerrainType vecino,
                         const char* nombre_base);


    void draw_sand_water_overlay(int col, int row, int tile_w, int tile_h, int tx, int ty,
                                 const ClientMap& client_map);
    static bool has_terrain(const ClientMap& map, int col, int row, TerrainType expected);
};

#endif
