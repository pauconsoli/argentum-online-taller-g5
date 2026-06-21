#include "player_renderer.h"

#include <string>
#include <unordered_map>

PlayerRenderer::PlayerRenderer(CharacterRenderer* character_renderer, SpriteManager* sprite_manager,
                               const Camera& camera):
    character_renderer_(character_renderer), sprite_manager_(sprite_manager), camera_(camera) {}

ItemType PlayerRenderer::get_item_type(const std::string& name) {
    static const std::unordered_map<std::string, ItemType> table = {
        {"Espada", ItemType::WEAPON},
        {"Hacha", ItemType::WEAPON},
        {"Martillo", ItemType::WEAPON},
        {"Arco simple", ItemType::WEAPON},
        {"Arco compuesto", ItemType::WEAPON},
        {"Vara de fresno", ItemType::STAFF},
        {"Báculo nudoso", ItemType::STAFF},
        {"Báculo engarzado", ItemType::STAFF},
        {"Flauta élfica", ItemType::STAFF},
        {"Armadura de cuero", ItemType::ARMOR},
        {"Armadura de placas", ItemType::ARMOR},
        {"Túnica azul", ItemType::ARMOR},
        {"Capucha", ItemType::HELMET},
        {"Casco de hierro", ItemType::HELMET},
        {"Sombrero mágico", ItemType::HELMET},
        {"Escudo de tortuga", ItemType::SHIELD},
        {"Escudo de hierro", ItemType::SHIELD},
    };
    auto it = table.find(name);
    return it != table.end() ? it->second : ItemType::OTHER;
}

uint16_t PlayerRenderer::head_index_for_race(uint8_t race) {
    switch (race) {
        case 1:
            return 101;  // ELF
        case 2:
            return 300;  // DWARF
        case 3:
            return 400;  // GNOME
        default:
            return 1;  // HUMAN
    }
}

void PlayerRenderer::render(const std::map<uint32_t, PlayerSnapshot>& players,
                            uint32_t my_player_id, uint32_t my_clan_id, int direction,
                            int current_frame,
                            const std::vector<InventorySlotData>& inventory_slots, int tile_w,
                            int tile_h) {
    for (const auto& [pid, ps] : players) {
        int px = camera_.get_screen_x(ps.x * tile_w);
        int py = camera_.get_screen_y(ps.y * tile_h);

        int body_x = px + (tile_w - CharacterRenderer::BODY_W) / 2;
        int body_y = py + tile_h - CharacterRenderer::BODY_H;

        SDL_Texture* body_texture = sprite_manager_->get_body(ps.race, ps.klass);
        SDL_Texture* head_texture = sprite_manager_->get_head(head_index_for_race(ps.race));

        int player_direction = (pid == my_player_id) ? direction : 0;
        int player_frame = (pid == my_player_id) ? current_frame : 0;

        character_renderer_->draw_character(body_texture, head_texture, body_x, body_y,
                                            player_direction, player_frame, ps.is_ghost);

        if (pid == my_player_id) {
            for (const auto& islot : inventory_slots) {
                if (!islot.is_equipped || islot.item_name.empty())
                    continue;
                ItemType itype = get_item_type(islot.item_name);
                SDL_Texture* itex =
                    sprite_manager_->get_item(SpriteManager::item_key_for_name(islot.item_name));
                if (!itex)
                    continue;
                character_renderer_->draw_equipped_item(body_x, body_y, itex, itype, tile_w);
            }
        }
        SDL_Color nick_color = {255, 255, 255, 255};
        if (my_clan_id != 0 && ps.clan_id == my_clan_id && pid != my_player_id)
            nick_color = {120, 255, 120, 255};
        character_renderer_->draw_nickname(ps.nick, body_x, body_y, nick_color);
    }
}
