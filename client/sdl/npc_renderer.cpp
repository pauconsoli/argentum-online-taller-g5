#include "npc_renderer.h"

// Dibuja todos los NPCs con su animación correspondiente; los humanoides también tienen cabeza.
void NPCRenderer::render_npcs(int tile_w, int tile_h) {
    // cambia cada 150
    const Uint32 npc_frame_delay = 150;
    for (auto& [nid, ns] : npcs_) {
        // nid es el id del npc, ns es su estado (posicion, tipo, direccion, etc)

        // y anim es el estado de animacion local (frame actual, ultima vez que cambio de frame, etc)
        auto& anim = npc_anim_states_[nid];

        // si no existe, lo crea
        SDL_Texture* npc_tex = sprite_manager->get_npc(anim.sprite_type);
        if (!npc_tex)
            continue;
        NPCSpriteInfo info = npc_sprite_info(anim.sprite_type);
        int dir = static_cast<int>(anim.direction);

        if (dir > 3)
            dir = 0;
        int max_frames = info.fpd[dir];

        // Avanza la animación sólo si el NPC está en movimiento.
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
        int px = camera.get_screen_x(ns.x * tile_w);
        int py = camera.get_screen_y(ns.y * tile_h);
        int dw = (info.draw_w > 0) ? info.draw_w : info.fw;
        int dh = (info.draw_h > 0) ? info.draw_h : info.fh;
        int body_x = px + (tile_w - dw) / 2;
        int body_y = py + tile_h - dh;
        renderer->draw_frame_scaled(npc_tex, frame * info.fw, dir * info.fh, info.fw, info.fh,
                                    body_x, body_y, dw, dh);

        // NPCs humanoides (banquero, sacerdote, mercader) tienen cabeza separada.
        if (info.head_index > 0) {
            static const int body_row_to_head_row[] = {2, 0, 3, 1};
            static const int head_y_offset[] = {-13, -19, -18, -17};
            constexpr int head_w = 27;
            constexpr int head_h = 64;
            SDL_Texture* head_tex = sprite_manager->get_head(info.head_index);
            if (head_tex) {
                renderer->draw_frame(head_tex, 0, body_row_to_head_row[dir] * head_h, head_w,
                                     head_h, body_x, body_y + head_y_offset[dir]);
            }
        }
    }
}

// datos de animacion x png
static NPCSpriteInfo npc_sprite_info(NPCVisualType t) {
    switch (t) {
        case NPCVisualType::BANKER:
            return {26, 46, {7, 7, 7, 7}};
        case NPCVisualType::PRIEST:
            return {27, 47, {6, 6, 5, 5}, 0, 0, 3};
        case NPCVisualType::MERCHANT:
            return {27, 47, {6, 6, 5, 5}, 0, 0, 30};
        case NPCVisualType::GOBLIN:
            return {32, 32, {6, 6, 6, 6}};
        case NPCVisualType::SKELETON:
            return {25, 52, {6, 6, 5, 5}};
        case NPCVisualType::ZOMBIE:
            return {128, 128, {6, 6, 6, 4}};
        case NPCVisualType::SPIDER:
            return {128, 128, {8, 8, 8, 8}};
        case NPCVisualType::ORC:
            return {24, 52, {6, 6, 5, 5}};
        case NPCVisualType::GOLEM_ICE:
            return {128, 128, {8, 8, 8, 8}, 64, 64};
        case NPCVisualType::GOLEM_STONE:
            return {128, 128, {8, 8, 8, 8}, 64, 64};
        case NPCVisualType::GOLEM_INFERNAL:
            return {128, 128, {8, 8, 8, 8}, 64, 64};
        default:
            return {32, 32, {1, 1, 1, 1}};
    }
}

// dimensiones del frame
struct NPCSpriteInfo {
    int fw;
    int fh;
    int fpd[4];
    int draw_w = 0;
    int draw_h = 0;
    int head_index = 0;
};
