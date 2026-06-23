#include "world_renderer.h"

#include <algorithm>
#include <string>
#include <vector>

#include <SDL2/SDL.h>

#include "../state/client_map.h"
#include "../state/sdl_game_state.h"

FringeEntry FringeEntry::make_tree(int col, int row) {
    FringeEntry entry{};
    entry.y_row = row;
    entry.type = FringeType::TREE;
    entry.col = col;
    return entry;
}
FringeEntry FringeEntry::make_item(const GroundItemSnapshot& ground_item) {
    FringeEntry entry{};
    entry.y_row = ground_item.y;
    entry.type = FringeType::ITEM;
    entry.item = &ground_item;
    return entry;
}
FringeEntry FringeEntry::make_player(uint32_t player_id, const PlayerSnapshot& player_snapshot) {
    FringeEntry entry{};
    entry.y_row = player_snapshot.y;
    entry.type = FringeType::PLAYER;
    entry.id = player_id;
    entry.ps = &player_snapshot;
    return entry;
}
FringeEntry FringeEntry::make_npc(uint32_t npc_id, const NPCSnapshot& npc_snapshot) {
    FringeEntry entry{};
    entry.y_row = npc_snapshot.y;
    entry.type = FringeType::NPC;
    entry.id = npc_id;
    entry.ns = &npc_snapshot;
    return entry;
}

WorldRenderer::WorldRenderer(Renderer& r, SpriteManager& sm, TerrainRenderer& tr, NPCRenderer& nr,
                             PlayerRenderer& pr, Hud& h, MiniChat& mc, HelpMenu& hm, ClanPanel& cp,
                             Camera& cam, const SdlConfig& cfg):
    renderer(r),
    sprite_manager(sm),
    terrain_renderer(tr),
    npc_renderer(nr),
    player_renderer(pr),
    hud(h),
    mini_chat(mc),
    help_menu(hm),
    clan_panel(cp),
    camera(cam),
    config(cfg) {}


TileBounds WorldRenderer::draw_terrain(const ClientMap& map, int screen_w, int screen_h) {
    TileBounds bounds;
    bounds.start_col = camera.get_x() / config.tile_width;
    bounds.end_col = bounds.start_col + screen_w / config.tile_width + 1;
    bounds.start_row = camera.get_y() / config.tile_height;
    bounds.end_row = bounds.start_row + screen_h / config.tile_height + 1;

    terrain_renderer.draw(bounds.start_col, bounds.end_col, bounds.start_row, bounds.end_row,
                          config.tile_width, config.tile_height, map, screen_w, screen_h);

    return bounds;
}

std::vector<FringeEntry> WorldRenderer::build_fringe(const TileBounds& bounds,
                                                     const GameState& state, const ClientMap& map) {
    std::vector<FringeEntry> fringe;
    fringe.reserve(64);

    for (int row = bounds.start_row; row <= bounds.end_row; ++row) {
        for (int col = bounds.start_col; col <= bounds.end_col; ++col) {
            if (col < 0 || row < 0 || col >= map.get_width() || row >= map.get_height())
                continue;
            const auto& tile = map.at(col, row);
            if (tile.blocking && tile.terrain == TerrainType::GRASS)
                fringe.push_back(FringeEntry::make_tree(col, row));
        }
    }

    std::transform(state.ground_items().begin(), state.ground_items().end(),
                   std::back_inserter(fringe), FringeEntry::make_item);

    for (const auto& [player_id, ps] : state.players())
        fringe.push_back(FringeEntry::make_player(player_id, ps));

    for (const auto& [npc_id, ns] : state.npcs())
        fringe.push_back(FringeEntry::make_npc(npc_id, ns));

    std::stable_sort(fringe.begin(), fringe.end(),
                     [](const FringeEntry& a, const FringeEntry& b) { return a.y_row < b.y_row; });

    return fringe;
}

// Dibuja un item tirado en el suelo (sombra + sprite con outline).
// Selección de textura: oro → item por nombre → sprite de fallback.
void WorldRenderer::draw_ground_item(const GroundItemSnapshot& ground_item) {
    SDL_Texture* tex =
        ground_item.is_gold ?
            sprite_manager.get_gold() :
            sprite_manager.get_item(SpriteManager::item_key_for_name(ground_item.name));
    if (tex == nullptr)
        tex = sprite_manager.get_item(FALLBACK_ITEM_SPRITE);
    if (tex == nullptr)
        return;

    int gx = camera.get_screen_x(ground_item.x * config.tile_width);
    int gy = camera.get_screen_y(ground_item.y * config.tile_height);
    int tw = config.tile_width;
    int th = config.tile_height;

    renderer.draw_shadow(gx + tw / 2, gy + th, static_cast<int>(tw * Renderer::SHADOW_WIDTH_RATIO));
    renderer.draw_frame_scaled_outlined(tex, 0, 0, tw, th, gx, gy, tw, th,
                                        Renderer::OUTLINE_THICKNESS);
}

void WorldRenderer::draw_fringe(const std::vector<FringeEntry>& fringe, const GameState& state,
                                int direction, int current_frame) {
    for (const auto& e : fringe) {
        switch (e.type) {
            case FringeType::TREE:
                terrain_renderer.draw_tree(e.col, e.y_row, config.tile_width, config.tile_height);
                break;
            case FringeType::ITEM:
                draw_ground_item(*e.item);
                break;
            case FringeType::PLAYER:
                player_renderer.render_single(e.id, *e.ps, state.player_id(), state.clan_id(),
                                              direction, current_frame, state.inventory_slots(),
                                              config.tile_width, config.tile_height);
                break;
            case FringeType::NPC:
                npc_renderer.render_single(e.id, *e.ns, config.tile_width, config.tile_height);
                break;
        }
    }
}

void WorldRenderer::draw_ui(const GameState& state, bool chat_active, const std::string& chat_input,
                            bool music_paused, int screen_h) {
    hud.draw(state.hp(), state.max_hp(), state.mp(), state.max_mp(), state.level(), state.gold(),
             state.xp());
    hud.draw_inventory(&sprite_manager, state.inventory_slots(), state.selected_slot());
    hud.draw_music_button(music_paused);
    mini_chat.draw();
    if (chat_active)
        mini_chat.draw_input(chat_input, screen_h - 30);
    help_menu.draw();
    clan_panel.draw();
}

void WorldRenderer::draw(const GameState& state, const ClientMap& map, int direction,
                         int current_frame, bool chat_active, const std::string& chat_input,
                         bool music_paused, int screen_w, int screen_h) {
    renderer.clear();
    const auto bounds = draw_terrain(map, screen_w, screen_h);
    const auto fringe = build_fringe(bounds, state, map);
    draw_fringe(fringe, state, direction, current_frame);
    draw_ui(state, chat_active, chat_input, music_paused, screen_h);
    renderer.present();
}
