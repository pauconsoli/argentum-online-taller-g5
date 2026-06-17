#ifndef TERRAIN_RENDERER_H
#define TERRAIN_RENDERER_H

#include "camera.h"
#include "client_map.h"
#include "common/world/terrain_type.h"
#include "renderer.h"
#include "sprite_manager.h"

class TerrainRenderer {
 public:
    TerrainRenderer(Renderer* renderer, SpriteManager* sprite_manager, const Camera& camera);

    void draw(int start_col, int end_col, int start_row, int end_row, int tile_w, int tile_h,
              const ClientMap& client_map);

 private:
    Renderer* renderer_;
    SpriteManager* sprite_manager_;
    const Camera& camera_;

    void draw_base_tiles(int start_col, int end_col, int start_row, int end_row, int tile_w,
                         int tile_h, const ClientMap& client_map);
    void draw_terrain_transitions(int start_col, int end_col, int start_row, int end_row,
                                  int tile_w, int tile_h, const ClientMap& client_map);

    void draw_base_tile(int col, int row, int tile_w, int tile_h, const ClientMap& client_map);
    void draw_overlays_for_tile(int col, int row, int tile_w, int tile_h,
                                const ClientMap& client_map);

    // Retorna false si (col,row) está fuera del mapa o si el terreno no coincide.
    static bool has_terrain(const ClientMap& map, int col, int row, TerrainType expected);
};

#endif
