#ifndef PLAYER_RENDERER_H
#define PLAYER_RENDERER_H

#include <cstdint>
#include <string>
#include <vector>

#include <SDL2/SDL.h>

#include "../assets/sprite_manager.h"
#include "../core/camera.h"
#include "character_renderer.h"
#include "common/updates/inventory_update.h"
#include "common/updates/snapshot_update.h"

class PlayerRenderer {
 public:
    PlayerRenderer(CharacterRenderer* character_renderer, SpriteManager* sprite_manager,
                   const Camera& camera);

    void render_single(uint32_t pid, const PlayerSnapshot& ps, uint32_t my_player_id,
                       uint32_t my_clan_id, int direction, int current_frame,
                       const std::vector<InventorySlotData>& inventory_slots, int tile_w,
                       int tile_h, Uint32 frame_delay_ms, int total_frames);

 private:
    static ItemType get_item_type(const std::string& name);
    static uint16_t head_index_for_race(uint8_t race);

    CharacterRenderer* character_renderer_;
    SpriteManager* sprite_manager_;
    const Camera& camera_;
};

#endif
