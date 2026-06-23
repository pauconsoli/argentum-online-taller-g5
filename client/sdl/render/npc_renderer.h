#ifndef NPC_RENDERER_H
#define NPC_RENDERER_H

#include <cstdint>
#include <map>
#include <vector>

#include <SDL2/SDL.h>

#include "../assets/sprite_manager.h"
#include "../core/camera.h"
#include "common/updates/snapshot_update.h"
#include "npc_visual_type.h"
#include "renderer.h"

class NPCRenderer {
 public:
    NPCRenderer(Renderer* renderer, SpriteManager* sprite_manager, const Camera& camera);

    void render_single(uint32_t nid, const NPCSnapshot& ns, int tile_w, int tile_h);
    void sync_from_snapshot(const std::vector<NPCSnapshot>& npcs);

 private:
    struct NPCSpriteInfo {
        int fw;
        int fh;
        int fpd[4];
        int draw_w = 0;
        int draw_h = 0;
        int head_index = 0;
    };
    struct NPCAnimState {
        int current_frame = 0;
        Uint32 last_frame_time = 0;
        NPCVisualType sprite_type = NPCVisualType::UNKNOWN;
        uint8_t direction = 0;
        bool is_moving = false;

        // Interpolation state: lerp_x/y track the current visual position (tile coords,
        // updated each frame so sync_from_snapshot can use them as the interp start).
        float lerp_x = 0.0f;
        float lerp_y = 0.0f;
        float prev_x = 0.0f;
        float prev_y = 0.0f;
        float target_x = 0.0f;
        float target_y = 0.0f;
        Uint32 interp_start = 0;
        bool position_initialized = false;
    };

    static NPCSpriteInfo npc_sprite_info(NPCVisualType t);

    Renderer* renderer_;
    SpriteManager* sprite_manager_;
    const Camera& camera_;
    std::map<uint32_t, NPCAnimState> npc_anim_states_;
};

#endif
