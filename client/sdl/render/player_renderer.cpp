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
    auto item = table.find(name);
    return item != table.end() ? item->second : ItemType::OTHER;
}

// Mapea ID de raza al índice base en el spritesheet de cabezas del SpriteManager.
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

void PlayerRenderer::render_single(uint32_t player_id, const PlayerSnapshot& player_snapshoot,
                                   uint32_t my_player_id, uint32_t my_clan_id, int direction,
                                   int current_frame,
                                   const std::vector<InventorySlotData>& inventory_slots,
                                   int tile_w, int tile_h) {

    int player_position_x = camera_.get_screen_x(player_snapshoot.x * tile_w);
    int player_position_y = camera_.get_screen_y(player_snapshoot.y * tile_h);

    int body_x = player_position_x + (tile_w - CharacterRenderer::BODY_W) / 2;
    int body_y = player_position_y + tile_h - CharacterRenderer::BODY_H;

    SDL_Texture* body_texture =
        sprite_manager_->get_body(player_snapshoot.race, player_snapshoot.klass);
    SDL_Texture* head_texture =
        sprite_manager_->get_head(head_index_for_race(player_snapshoot.race));

    int player_direction = (player_id == my_player_id) ? direction : 0;
    int player_xframe = (player_id == my_player_id) ? current_frame : 0;

    if (!player_snapshoot.is_ghost) {
        int shadow_w = static_cast<int>(tile_w * Renderer::SHADOW_WIDTH_RATIO);
        character_renderer_->draw_shadow(player_position_x + tile_w / 2, player_position_y + tile_h,
                                         shadow_w);
    }

    character_renderer_->draw_character(body_texture, head_texture, body_x, body_y,
                                        player_direction, player_xframe, player_snapshoot.is_ghost);

    if (player_id == my_player_id) {
        for (const auto& islot : inventory_slots) {
            if (!islot.is_equipped || islot.item_name.empty())
                continue;
            ItemType item_type = get_item_type(islot.item_name);
            SDL_Texture* item_texture =
                sprite_manager_->get_item(SpriteManager::item_key_for_name(islot.item_name));
            if (!item_texture)
                continue;
            character_renderer_->draw_equipped_item(body_x, body_y, item_texture, item_type,
                                                    tile_w);
        }
    }

    // Blanco por defecto; verde si el otro jugador pertenece al mismo clan.
    SDL_Color nick_color = {255, 255, 255, 255};
    if (my_clan_id != 0 && player_snapshoot.clan_id == my_clan_id && player_id != my_player_id)
        nick_color = {120, 255, 120, 255};
    character_renderer_->draw_nickname(player_snapshoot.nick, body_x, body_y, nick_color);
}
