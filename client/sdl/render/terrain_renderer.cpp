#include "terrain_renderer.h"

#include <cstdio>

TerrainRenderer::TerrainRenderer(Renderer* renderer, SpriteManager* sprite_manager,
                                 const Camera& camera):
    renderer_(renderer), sprite_manager_(sprite_manager), camera_(camera) {}

TerrainRenderer::~TerrainRenderer() {
    if (terrain_cache_)
        SDL_DestroyTexture(terrain_cache_);
}

TerrainRenderer::Cardinals TerrainRenderer::cardinals_of(const ClientMap& map, int col, int row,
                                                         TerrainType t) {
    return {has_terrain(map, col, row - 1, t), has_terrain(map, col, row + 1, t),
            has_terrain(map, col + 1, row, t), has_terrain(map, col - 1, row, t)};
}

void TerrainRenderer::draw(int start_col, int end_col, int start_row, int end_row, int tile_w,
                           int tile_h, const ClientMap& client_map, int screen_w, int screen_h) {
    if (cache_is_old(start_col, start_row, screen_w, screen_h))
        rebuild_cache(start_col, end_col, start_row, end_row, tile_w, tile_h, client_map, screen_w,
                      screen_h);

    SDL_RenderCopy(renderer_->get_sdl_renderer(), terrain_cache_, nullptr, nullptr);
}

bool TerrainRenderer::cache_is_old(int start_col, int start_row, int screen_w, int screen_h) const {
    bool size_changed = (cache_w_ != screen_w || cache_h_ != screen_h);
    bool pos_changed = (cache_col_ != start_col || cache_row_ != start_row);
    return terrain_cache_ == nullptr || size_changed || pos_changed;
}

void TerrainRenderer::rebuild_cache(int start_col, int end_col, int start_row, int end_row,
                                    int tile_w, int tile_h, const ClientMap& client_map,
                                    int screen_w, int screen_h) {
    SDL_Renderer* sdl_r = renderer_->get_sdl_renderer();

    if (terrain_cache_ == nullptr || cache_w_ != screen_w || cache_h_ != screen_h) {
        if (terrain_cache_)
            SDL_DestroyTexture(terrain_cache_);
        terrain_cache_ = SDL_CreateTexture(sdl_r, SDL_PIXELFORMAT_RGBA8888,
                                           SDL_TEXTUREACCESS_TARGET, screen_w, screen_h);
    }

    SDL_SetRenderTarget(sdl_r, terrain_cache_);
    SDL_SetRenderDrawColor(sdl_r, 0, 0, 0, 255);
    SDL_RenderClear(sdl_r);

    draw_base_tiles(start_col, end_col, start_row, end_row, tile_w, tile_h, client_map);
    draw_terrain_transitions(start_col, end_col, start_row, end_row, tile_w, tile_h, client_map);
    SDL_SetRenderTarget(sdl_r, nullptr);

    cache_col_ = start_col;
    cache_row_ = start_row;
    cache_w_ = screen_w;
    cache_h_ = screen_h;
}


void TerrainRenderer::draw_base_tiles(int start_col, int end_col, int start_row, int end_row,
                                      int tile_w, int tile_h, const ClientMap& client_map) {
    for (int row = start_row; row <= end_row; row++) {
        for (int col = start_col; col <= end_col; col++) {
            draw_base_tile(col, row, tile_w, tile_h, client_map);
        }
    }
}

void TerrainRenderer::draw_terrain_transitions(int start_col, int end_col, int start_row,
                                               int end_row, int tile_w, int tile_h,
                                               const ClientMap& client_map) {
    for (int row = start_row; row <= end_row; row++) {
        for (int col = start_col; col <= end_col; col++) {
            bool in_bounds = (col >= 0 && col < client_map.get_width() && row >= 0 &&
                              row < client_map.get_height());
            if (!in_bounds)
                continue;
            draw_overlays_for_tile(col, row, tile_w, tile_h, client_map);
        }
    }
}

void TerrainRenderer::draw_base_tile(int col, int row, int tile_w, int tile_h,
                                     const ClientMap& client_map) {
    int tx = camera_.get_screen_x(col * tile_w);
    int ty = camera_.get_screen_y(row * tile_h);
    bool in_bounds =
        (col >= 0 && col < client_map.get_width() && row >= 0 && row < client_map.get_height());
    TerrainType terrain = in_bounds ? client_map.at(col, row).terrain : TerrainType::GRASS;

    renderer_->draw_frame(sprite_manager_->get_terrain(terrain), 0, 0, tile_w, tile_h, tx, ty);
}

void TerrainRenderer::draw_overlays_for_tile(int col, int row, int tile_w, int tile_h,
                                             const ClientMap& client_map) {
    int tx = camera_.get_screen_x(col * tile_w);
    int ty = camera_.get_screen_y(row * tile_h);
    TerrainType terrain = client_map.at(col, row).terrain;

    bool is_dungeon_entrance =
        (terrain == TerrainType::DUNGEON_ENTRANCE_1 || terrain == TerrainType::DUNGEON_ENTRANCE_2);
    SDL_Texture* type_overlay =
        is_dungeon_entrance ? nullptr : sprite_manager_->get_terrain_overlay(terrain);

    if (type_overlay) {
        int ow, oh;
        SDL_QueryTexture(type_overlay, nullptr, nullptr, &ow, &oh);
        renderer_->draw_frame(type_overlay, 0, 0, ow, oh, tx + (tile_w - ow) / 2,
                              ty + (tile_h - oh));
    }

    draw_sand_water_overlay(col, row, tile_w, tile_h, tx, ty, client_map);
    draw_transition(col, row, tile_w, tile_h, tx, ty, client_map, TerrainType::WATER,
                    TerrainType::GRASS, "terrain_overlay_grass_water");
    draw_transition(col, row, tile_w, tile_h, tx, ty, client_map, TerrainType::DIRT,
                    TerrainType::GRASS, "terrain_overlay_grass");
    draw_transition(col, row, tile_w, tile_h, tx, ty, client_map, TerrainType::SAND,
                    TerrainType::GRASS, "terrain_overlay_grass");
    draw_transition(col, row, tile_w, tile_h, tx, ty, client_map, TerrainType::STONE,
                    TerrainType::GRASS, "terrain_overlay_grass");
}


void TerrainRenderer::draw_transition(int col, int row, int tile_w, int tile_h, int tx, int ty,
                                      const ClientMap& client_map, TerrainType sobre,
                                      TerrainType vecino, const char* nombre_base) {

    if (client_map.at(col, row).terrain != sobre)
        return;

    Cardinals neighbors = cardinals_of(client_map, col, row, vecino);

    char key_edge[64];
    char key_corner[64];
    std::snprintf(key_edge, sizeof(key_edge), "%s_edge", nombre_base);
    std::snprintf(key_corner, sizeof(key_corner), "%s_corner", nombre_base);

    SDL_Texture* edge_tex = sprite_manager_->get_transition_overlay(key_edge);
    SDL_Texture* corner_tex = sprite_manager_->get_transition_overlay(key_corner);

    auto draw_edge = [&](double rotation_angle) {
        if (edge_tex)
            renderer_->draw_frame_rotated(edge_tex, 0, 0, tile_w, tile_h, tx, ty, rotation_angle);
    };

    // rotar
    auto corner_if = [&](bool has_north_or_south_neighbor, bool has_east_or_west_neighbor,
                         int delta_col, int delta_row, double rotation_angle) {
        bool solo_en_diagonal = !has_north_or_south_neighbor && !has_east_or_west_neighbor;
        bool hay_vecino_diagonal =
            has_terrain(client_map, col + delta_col, row + delta_row, vecino);

        if (solo_en_diagonal && hay_vecino_diagonal && corner_tex)
            renderer_->draw_frame_rotated(corner_tex, 0, 0, tile_w, tile_h, tx, ty, rotation_angle);
    };

    if (neighbors.north)
        draw_edge(0.0);  // norte
    if (neighbors.east)
        draw_edge(90.0);  // este
    if (neighbors.south)
        draw_edge(180.0);  // sur
    if (neighbors.west)
        draw_edge(270.0);  // oeste

    corner_if(neighbors.north, neighbors.west, -1, -1, 0.0);    // esquina noroeste
    corner_if(neighbors.north, neighbors.east, +1, -1, 90.0);   // esquina noreste
    corner_if(neighbors.south, neighbors.east, +1, +1, 180.0);  // esquina sureste
    corner_if(neighbors.south, neighbors.west, -1, +1, 270.0);  // esquina suroeste
}


void TerrainRenderer::draw_tree(int col, int row, int tile_w, int tile_h) {
    int tx = camera_.get_screen_x(col * tile_w);
    int ty = camera_.get_screen_y(row * tile_h);
    int shadow_w = static_cast<int>(tile_w * Renderer::SHADOW_TREE_RATIO);
    renderer_->draw_shadow(tx + tile_w / 2, ty + tile_h, shadow_w);
    renderer_->draw_frame(sprite_manager_->get_tree(), 0, 0, tile_w, tile_h, tx, ty);
}

void TerrainRenderer::draw_sand_water_overlay(int col, int row, int tile_w, int tile_h, int tx,
                                              int ty, const ClientMap& client_map) {
    if (client_map.at(col, row).terrain != TerrainType::WATER)
        return;

    Cardinals s = cardinals_of(client_map, col, row, TerrainType::SAND);
    int mask = (s.north << 3) | (s.south << 2) | (s.east << 1) | s.west;

    const char* key = nullptr;
    switch (mask) {
        case 0b0101:
            key = "ao_angulo_ne";
            break;
        case 0b1010:
            key = "ao_angulo_so";
            break;
        case 0b0110:
            key = "ao_angulo_se";
            break;
        case 0b1001:
            key = "ao_angulo_nw";
            break;
        case 0b0100:
            key = "ao_costa_norte";
            break;
        case 0b1000:
            key = "ao_costa_sur";
            break;
        case 0b0001:
            key = "ao_costa_este";
            break;
        case 0b0010:
            key = "ao_costa_oeste";
            break;
        default:
            break;
    }

    if (!key)
        return;
    SDL_Texture* tex = sprite_manager_->get_transition_overlay(key);
    if (tex)
        renderer_->draw_frame(tex, 0, 0, tile_w, tile_h, tx, ty);
}


bool TerrainRenderer::has_terrain(const ClientMap& map, int col, int row, TerrainType expected) {
    if (col < 0 || col >= map.get_width() || row < 0 || row >= map.get_height())
        return false;
    return map.at(col, row).terrain == expected;
}
