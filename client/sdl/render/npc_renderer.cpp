#include "npc_renderer.h"

#include <algorithm>

#include <SDL2/SDL.h>

NPCRenderer::NPCRenderer(Renderer* renderer, SpriteManager* sprite_manager, const Camera& camera):
    renderer_(renderer), sprite_manager_(sprite_manager), camera_(camera) {}

void NPCRenderer::sync_from_snapshot(const std::vector<NPCSnapshot>& npcs) {
    static const uint8_t sprite_dir[] = {1, 0, 2, 3};

    for (const auto& npc_snapshoot : npcs) {

        auto& animation = npc_anim_states_[npc_snapshoot.npc_id];

        NPCVisualType new_type = npc_visual_type_from_network(npc_snapshoot.npc_type);
        if (animation.sprite_type != new_type) {
            animation.sprite_type = new_type;
            animation.current_frame = 0;
        }
        uint8_t new_dir = (npc_snapshoot.direction < 4) ? sprite_dir[npc_snapshoot.direction] : 0;
        if (animation.direction != new_dir) {
            animation.direction = new_dir;
            animation.current_frame = 0;
        }
        animation.is_moving = npc_snapshoot.is_moving;
    }
    for (auto dead_npc_it = npc_anim_states_.begin(); dead_npc_it != npc_anim_states_.end();) {
        bool found = std::any_of(npcs.begin(), npcs.end(), [&](const auto& npc_snapshoot) {
            return npc_snapshoot.npc_id == dead_npc_it->first;
        });
        if (!found)
            dead_npc_it = npc_anim_states_.erase(dead_npc_it);
        else
            ++dead_npc_it;
    }
}

void NPCRenderer::render_single(uint32_t npc_id, const NPCSnapshot& ns, int tile_w, int tile_h) {
    const Uint32 npc_frame_delay = 100;
    auto& animation = npc_anim_states_[npc_id];  // snapshoot

    SDL_Texture* npc_tex = sprite_manager_->get_npc(animation.sprite_type);
    if (!npc_tex)
        return;

    NPCSpriteInfo info = npc_sprite_info(animation.sprite_type);
    int dir = static_cast<int>(animation.direction);

    if (dir > 3)
        dir = 0;
    int max_frames = info.frame_direction[dir];

    if (animation.is_moving) {
        Uint32 now = SDL_GetTicks();
        if (now - animation.last_frame_time > npc_frame_delay) {
            animation.current_frame = (animation.current_frame + 1) % max_frames;
            animation.last_frame_time = now;
        }
    } else {
        animation.current_frame = 0;
    }

    int frame = animation.current_frame % max_frames;

    int position_x = camera_.get_screen_x(static_cast<int>(ns.x) * tile_w);
    int position_y = camera_.get_screen_y(static_cast<int>(ns.y) * tile_h);

    int draw_w = (info.draw_w > 0) ? info.draw_w : info.frame_sprite_w;
    int draw_h = (info.draw_h > 0) ? info.draw_h : info.frame_sprite_h;
    int body_x = position_x + (tile_w - draw_w) / 2;
    int body_y = position_y + tile_h - draw_h;


    int shadow_w = std::max(static_cast<int>(tile_w * Renderer::SHADOW_WIDTH_RATIO),
                            static_cast<int>(draw_w * Renderer::SHADOW_LARGE_NPC_RATIO));
    renderer_->draw_shadow(position_x + tile_w / 2, position_y + tile_h, shadow_w);
    renderer_->draw_frame_scaled_outlined(
        npc_tex, frame * info.frame_sprite_w, dir * info.frame_sprite_h, info.frame_sprite_w,
        info.frame_sprite_h, body_x, body_y, draw_w, draw_h, Renderer::OUTLINE_THICKNESS);

    if (info.head_index > 0) {
        static const int body_row_to_head_row[] = {2, 0, 3, 1};
        static const int head_y_offset[] = {-20, -19, -18, -17};
        constexpr int head_w = 27;
        constexpr int head_h = 64;
        SDL_Texture* head_tex = sprite_manager_->get_head(info.head_index);
        if (head_tex) {
            renderer_->draw_frame(head_tex, 0, body_row_to_head_row[dir] * head_h, head_w, head_h,
                                  body_x, body_y + head_y_offset[dir]);
        }
    }
}

NPCRenderer::NPCSpriteInfo NPCRenderer::npc_sprite_info(NPCVisualType t) {
    switch (t) {
        case NPCVisualType::BANKER:
            return {28, 49, {7, 7, 7, 7}};
        case NPCVisualType::PRIEST:
            return {28, 49, {6, 6, 5, 5}, 0, 0, 3};
        case NPCVisualType::MERCHANT:
            return {28, 49, {6, 6, 5, 6}, 0, 0, 30};
        case NPCVisualType::GOBLIN:
            return {24, 32, {8, 8, 8, 8}};
        case NPCVisualType::SKELETON:
            return {25, 52, {6, 6, 5, 5}};
        case NPCVisualType::ZOMBIE:
            return {96, 96, {8, 8, 8, 8}};
        case NPCVisualType::SPIDER:
            return {128, 128, {8, 8, 8, 8}};
        case NPCVisualType::ORC:
            return {24, 52, {6, 6, 5, 5}};
        case NPCVisualType::GOLEM_ICE:
            return {160, 160, {6, 6, 6, 6}};
        case NPCVisualType::GOLEM_STONE:
            return {160, 160, {6, 6, 6, 6}};
        case NPCVisualType::GOLEM_INFERNAL:
            return {160, 160, {6, 6, 6, 6}};
        default:
            return {32, 32, {1, 1, 1, 1}};
    }
}
