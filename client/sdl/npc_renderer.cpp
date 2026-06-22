#include "npc_renderer.h"

#include <algorithm>
#include <cmath>

#include <SDL2/SDL.h>

NPCRenderer::NPCRenderer(Renderer* renderer, SpriteManager* sprite_manager, const Camera& camera):
    renderer_(renderer), sprite_manager_(sprite_manager), camera_(camera) {}

void NPCRenderer::sync_from_snapshot(const std::vector<NPCSnapshot>& npcs) {
    static const uint8_t kDirToRow[] = {1, 0, 2, 3};
    for (const auto& ns : npcs) {
        auto& anim = npc_anim_states_[ns.npc_id];
        NPCVisualType new_type = npc_visual_type_from_network(ns.npc_type);
        if (anim.sprite_type != new_type) {
            anim.sprite_type = new_type;
            anim.current_frame = 0;
        }
        uint8_t new_dir = (ns.direction < 4) ? kDirToRow[ns.direction] : 0;
        if (anim.direction != new_dir) {
            anim.direction = new_dir;
            anim.current_frame = 0;
        }
        anim.is_moving = ns.is_moving;

        float new_x = static_cast<float>(ns.x);
        float new_y = static_cast<float>(ns.y);
        if (!anim.position_initialized) {
            anim.lerp_x = anim.prev_x = anim.target_x = new_x;
            anim.lerp_y = anim.prev_y = anim.target_y = new_y;
            anim.interp_start = SDL_GetTicks();
            anim.position_initialized = true;
        } else if (anim.target_x != new_x || anim.target_y != new_y) {
            anim.prev_x = anim.lerp_x;
            anim.prev_y = anim.lerp_y;
            anim.target_x = new_x;
            anim.target_y = new_y;
            anim.interp_start = SDL_GetTicks();
        }
    }
    for (auto it = npc_anim_states_.begin(); it != npc_anim_states_.end();) {
        bool found = std::any_of(npcs.begin(), npcs.end(),
                                 [&](const auto& ns) { return ns.npc_id == it->first; });
        if (!found)
            it = npc_anim_states_.erase(it);
        else
            ++it;
    }
}

void NPCRenderer::render_single(uint32_t nid, [[maybe_unused]] const NPCSnapshot& ns, int tile_w,
                                int tile_h) {
    const Uint32 npc_frame_delay = 150;
    auto& anim = npc_anim_states_[nid];
    SDL_Texture* npc_tex = sprite_manager_->get_npc(anim.sprite_type);
    if (!npc_tex)
        return;
    NPCSpriteInfo info = npc_sprite_info(anim.sprite_type);
    int dir = static_cast<int>(anim.direction);
    if (dir > 3)
        dir = 0;
    int max_frames = info.fpd[dir];

    if (anim.is_moving) {
        Uint32 now = SDL_GetTicks();
        if (now - anim.last_frame_time > npc_frame_delay) {
            anim.current_frame = (anim.current_frame + 1) % max_frames;
            anim.last_frame_time = now;
        }
    } else {
        anim.current_frame = 0;
    }

    int frame = anim.current_frame % max_frames;

    constexpr float kInterpDurationMs = 100.0f;
    Uint32 now_pos = SDL_GetTicks();
    float t = std::min(1.0f, static_cast<float>(now_pos - anim.interp_start) / kInterpDurationMs);
    anim.lerp_x = anim.prev_x + (anim.target_x - anim.prev_x) * t;
    anim.lerp_y = anim.prev_y + (anim.target_y - anim.prev_y) * t;
    int px = camera_.get_screen_x(static_cast<int>(std::lround(anim.lerp_x * tile_w)));
    int py = camera_.get_screen_y(static_cast<int>(std::lround(anim.lerp_y * tile_h)));
    int dw = (info.draw_w > 0) ? info.draw_w : info.fw;
    int dh = (info.draw_h > 0) ? info.draw_h : info.fh;
    int body_x = px + (tile_w - dw) / 2;
    int body_y = py + tile_h - dh;

    // Sombra debajo del NPC: base en tile_w para tamaño uniforme entre criaturas.
    // El max() evita que sprites grandes (golems 64px, zombies 128px) queden
    // con sombra más chica que un goblin de 32px.
    int shadow_w = std::max(static_cast<int>(tile_w * Renderer::SHADOW_WIDTH_RATIO),
                            static_cast<int>(dw * Renderer::SHADOW_LARGE_NPC_RATIO));
    renderer_->draw_shadow(px + tile_w / 2, py + tile_h, shadow_w);

    renderer_->draw_frame_scaled_outlined(npc_tex, frame * info.fw, dir * info.fh, info.fw, info.fh,
                                          body_x, body_y, dw, dh, Renderer::OUTLINE_THICKNESS);

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

void NPCRenderer::render(const std::map<uint32_t, NPCSnapshot>& npcs, int tile_w, int tile_h) {
    for (const auto& [nid, ns] : npcs) render_single(nid, ns, tile_w, tile_h);
}

NPCRenderer::NPCSpriteInfo NPCRenderer::npc_sprite_info(NPCVisualType t) {
    // fw/fh medidos del PNG: PNG_W/ncols y PNG_H/4filas.
    // draw_w/draw_h: tamaño de dibujo en pantalla (0 = sin escalar).
    // fpd: frames por dirección verificados contra contenido real del spritesheet.
    switch (t) {
        case NPCVisualType::BANKER:
            // 196x196 -> 7 cols x 4 filas
            return {28, 49, {7, 7, 7, 7}};
        case NPCVisualType::PRIEST:
            // 168x196 -> 6 cols x 4 filas
            return {28, 49, {6, 6, 5, 5}, 0, 0, 3};
        case NPCVisualType::MERCHANT:
            // 168x196 -> 6 cols x 4 filas, dir3 tiene 6 frames
            return {28, 49, {6, 6, 5, 6}, 0, 0, 30};
        case NPCVisualType::GOBLIN:
            // 256x128 -> 8 frames x 4 dirs, fw=24 fh=32; fw=32 incluia frame adyacente
            return {24, 32, {8, 8, 8, 8}};
        case NPCVisualType::SKELETON:
            // 150x208 -> 6 cols x 4 filas, fw=25 fh=52
            return {25, 52, {6, 6, 5, 5}};
        case NPCVisualType::ZOMBIE:
            // Extraído de Recursos/Graficos/4795.png filas 0-3 (de 8 totales)
            // fw=96, fh=96 verificados píxel a píxel; 8 frames por dirección
            // PNG resultante: 1024x384 RGBA
            return {96, 96, {8, 8, 8, 8}};
        case NPCVisualType::SPIDER:
            // Extraído de Recursos/Graficos/4792.png filas 0-3 (de 8 totales)
            // fw=128, fh=128 verificados píxel a píxel; 8 frames por dirección
            // PNG resultante: 1024x512 RGBA
            return {128, 128, {8, 8, 8, 8}};
        case NPCVisualType::ORC:
            // 144x208 -> 6 cols x 4 filas, fw=24 fh=52
            return {24, 52, {6, 6, 5, 5}};
        case NPCVisualType::GOLEM_ICE:
            // Extraído de Recursos/Graficos/4592.png filas 0-3 (de 4 útiles)
            // fw=160, fh=160 verificados píxel a píxel; 6 frames por dirección
            // PNG resultante: 1024x640 RGBA
            return {160, 160, {6, 6, 6, 6}};
        case NPCVisualType::GOLEM_STONE:
            // Extraído de Recursos/Graficos/4580.png filas 0-3 (de 4 útiles)
            // fw=160, fh=160 verificados píxel a píxel; 6 frames por dirección
            // PNG resultante: 1024x640 RGBA
            return {160, 160, {6, 6, 6, 6}};
        case NPCVisualType::GOLEM_INFERNAL:
            // Extraído de Recursos/Graficos/4593.png filas 0-3 (de 4 útiles)
            // fw=160, fh=160 verificados píxel a píxel; 6 frames por dirección
            // PNG resultante: 1024x640 RGBA
            return {160, 160, {6, 6, 6, 6}};
        default:
            return {32, 32, {1, 1, 1, 1}};
    }
}
