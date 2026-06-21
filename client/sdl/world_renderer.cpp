#include "world_renderer.h"

#include <algorithm>
#include <functional>
#include <iterator>
#include <string>
#include <vector>

#include <SDL2/SDL.h>

#include "client_map.h"
#include "sdl_game_state.h"

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

void WorldRenderer::draw(const GameState& state, const ClientMap& map, int direction,
                         int current_frame, bool chat_active, const std::string& chat_input,
                         bool music_paused, int screen_w, int screen_h) {
    renderer.clear();

    int start_col = camera.get_x() / config.tile_width;
    int end_col = start_col + screen_w / config.tile_width + 1;
    int start_row = camera.get_y() / config.tile_height;
    int end_row = start_row + screen_h / config.tile_height + 1;
    terrain_renderer.draw(start_col, end_col, start_row, end_row, config.tile_width,
                          config.tile_height, map);

    // --- CAPA FRINGE: árboles, items, jugadores y NPCs ordenados por Y ---
    struct FringeItem {
        int y_row;
        std::function<void()> draw;
    };
    std::vector<FringeItem> fringe;
    fringe.reserve(64);

    // Árboles (tiles blocking de pasto) — sacados del terrain pass para poder Y-sortear
    for (int row = start_row; row <= end_row; ++row) {
        for (int col = start_col; col <= end_col; ++col) {
            if (col < 0 || col >= map.get_width() || row < 0 || row >= map.get_height())
                continue;
            const auto& tile = map.at(col, row);
            if (tile.blocking && tile.terrain == TerrainType::GRASS) {
                fringe.push_back({row, [this, col, row]() {
                                      terrain_renderer.draw_tree(col, row, config.tile_width,
                                                                 config.tile_height);
                                  }});
            }
        }
    }

    // Items en el suelo
    std::transform(
        state.ground_items().begin(), state.ground_items().end(), std::back_inserter(fringe),
        [this](const auto& gi) -> FringeItem {
            return {gi.y, [this, gi]() {
                        SDL_Texture* item_tex = nullptr;
                        if (gi.is_gold) {
                            item_tex = sprite_manager.get_gold();
                        } else {
                            std::string item_key = SpriteManager::item_key_for_name(gi.name);
                            item_tex = sprite_manager.get_item(item_key);
                        }
                        if (item_tex == nullptr)
                            item_tex = sprite_manager.get_item("item_espada");
                        if (item_tex != nullptr) {
                            int gx = camera.get_screen_x(gi.x * config.tile_width);
                            int gy = camera.get_screen_y(gi.y * config.tile_height);
                            int tw = config.tile_width;
                            int th = config.tile_height;
                            renderer.draw_shadow(
                                gx + tw / 2, gy + th,
                                static_cast<int>(tw * Renderer::SHADOW_WIDTH_RATIO));
                            renderer.draw_frame_scaled_outlined(item_tex, 0, 0, tw, th, gx, gy, tw,
                                                                th, Renderer::OUTLINE_THICKNESS);
                        }
                    }};
        });

    // Jugadores
    for (const auto& [pid, ps] : state.players()) {
        fringe.push_back({ps.y, [this, pid, &ps, direction, current_frame, &state]() {
                              player_renderer.render_single(pid, ps, state.player_id(),
                                                            state.clan_id(), direction,
                                                            current_frame, state.inventory_slots(),
                                                            config.tile_width, config.tile_height);
                          }});
    }

    // NPCs
    for (const auto& [nid, ns] : state.npcs()) {
        fringe.push_back({ns.y, [this, nid, &ns]() {
                              npc_renderer.render_single(nid, ns, config.tile_width,
                                                         config.tile_height);
                          }});
    }

    std::stable_sort(fringe.begin(), fringe.end(),
                     [](const FringeItem& a, const FringeItem& b) { return a.y_row < b.y_row; });
    for (auto& item : fringe) item.draw();

    hud.draw(state.hp(), state.max_hp(), state.mp(), state.max_mp(), state.level(), state.gold(),
             state.xp());
    hud.draw_inventory(&sprite_manager, state.inventory_slots(), state.selected_slot());
    hud.draw_music_button(music_paused);

    mini_chat.draw();
    if (chat_active)
        mini_chat.draw_input(chat_input, screen_h - 30);
    help_menu.draw();
    clan_panel.draw();
    renderer.present();
}
