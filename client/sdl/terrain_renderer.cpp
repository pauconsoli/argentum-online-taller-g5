#include "terrain_renderer.h"

TerrainRenderer::TerrainRenderer(Renderer* renderer, SpriteManager* sprite_manager,
                                 const Camera& camera):
    renderer_(renderer), sprite_manager_(sprite_manager), camera_(camera) {}

void TerrainRenderer::draw(int start_col, int end_col, int start_row, int end_row, int tile_w,
                           int tile_h, const ClientMap& client_map) {
    draw_base_tiles(start_col, end_col, start_row, end_row, tile_w, tile_h, client_map);
    draw_terrain_transitions(start_col, end_col, start_row, end_row, tile_w, tile_h, client_map);
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
    bool blocking = in_bounds && client_map.at(col, row).blocking;
    renderer_->draw_frame(sprite_manager_->get_terrain(terrain), 0, 0, tile_w, tile_h, tx, ty);
    // Árbol: se escala al tamaño del tile, igual que en el código original.
    if (blocking && terrain == TerrainType::GRASS) {
        renderer_->draw_frame(sprite_manager_->get_tree(), 0, 0, tile_w, tile_h, tx, ty);
    }
}

void TerrainRenderer::draw_overlays_for_tile(int col, int row, int tile_w, int tile_h,
                                             const ClientMap& client_map) {
    int tx = camera_.get_screen_x(col * tile_w);
    int ty = camera_.get_screen_y(row * tile_h);
    TerrainType terrain = client_map.at(col, row).terrain;

    // Overlays de tipo (entradas de mazmorras, muros de ciudad).
    // Pueden ser más altos que un tile: se centran horizontal y se anclan al fondo.
    SDL_Texture* type_overlay = sprite_manager_->get_terrain_overlay(terrain);
    if (type_overlay) {
        int ow, oh;
        SDL_QueryTexture(type_overlay, nullptr, nullptr, &ow, &oh);
        renderer_->draw_frame(type_overlay, 0, 0, ow, oh, tx + (tile_w - ow) / 2,
                              ty + (tile_h - oh));
    }

    // Overlays de transición agua–arena sobre tiles de ARENA.
    // Se usan dos PNGs base con rotación clockwise para cubrir las 8 direcciones:
    //   edge:   agua en borde norte → 0°=N, 90°=E, 180°=S, 270°=W
    //   corner: agua en esquina NW  → 0°=NW, 90°=NE, 180°=SE, 270°=SW
    if (terrain != TerrainType::SAND)
        return;

    bool water_n = has_terrain(client_map, col, row - 1, TerrainType::WATER);
    bool water_s = has_terrain(client_map, col, row + 1, TerrainType::WATER);
    bool water_e = has_terrain(client_map, col + 1, row, TerrainType::WATER);
    bool water_w = has_terrain(client_map, col - 1, row, TerrainType::WATER);

    SDL_Texture* edge_tex =
        sprite_manager_->get_transition_overlay("terrain_overlay_water_sand_edge");
    SDL_Texture* corner_tex =
        sprite_manager_->get_transition_overlay("terrain_overlay_water_sand_corner");

    auto draw_edge = [&](double angle_deg) {
        if (edge_tex)
            renderer_->draw_frame_rotated(edge_tex, 0, 0, tile_w, tile_h, tx, ty, angle_deg);
    };
    auto draw_corner = [&](double angle_deg) {
        if (corner_tex)
            renderer_->draw_frame_rotated(corner_tex, 0, 0, tile_w, tile_h, tx, ty, angle_deg);
    };

    if (water_n)
        draw_edge(0.0);
    if (water_e)
        draw_edge(90.0);
    if (water_s)
        draw_edge(180.0);
    if (water_w)
        draw_edge(270.0);

    // Esquinas interiores: agua solo en diagonal (sin agua en los lados adyacentes).
    if (!water_n && !water_w && has_terrain(client_map, col - 1, row - 1, TerrainType::WATER))
        draw_corner(0.0);
    if (!water_n && !water_e && has_terrain(client_map, col + 1, row - 1, TerrainType::WATER))
        draw_corner(90.0);
    if (!water_s && !water_e && has_terrain(client_map, col + 1, row + 1, TerrainType::WATER))
        draw_corner(180.0);
    if (!water_s && !water_w && has_terrain(client_map, col - 1, row + 1, TerrainType::WATER))
        draw_corner(270.0);
}

bool TerrainRenderer::has_terrain(const ClientMap& map, int col, int row, TerrainType expected) {
    if (col < 0 || col >= map.get_width() || row < 0 || row >= map.get_height())
        return false;
    return map.at(col, row).terrain == expected;
}
