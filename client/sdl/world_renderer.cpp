#include "world_renderer.h"

#include <string>

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

    for (const auto& gi : state.ground_items()) {
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
            renderer.draw_frame(item_tex, 0, 0, config.tile_width, config.tile_height, gx, gy);
        }
    }

    player_renderer.render(state.players(), state.player_id(), state.clan_id(), direction,
                           current_frame, state.inventory_slots(), config.tile_width,
                           config.tile_height);
    npc_renderer.render(state.npcs(), config.tile_width, config.tile_height);

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
