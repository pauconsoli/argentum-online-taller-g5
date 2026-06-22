#include "character_renderer.h"

CharacterRenderer::CharacterRenderer(Renderer* renderer): renderer(renderer) {}

void CharacterRenderer::draw_character(SDL_Texture* body_tex, SDL_Texture* head_tex,
                                       int player_position_x, int player_position_y, int direction,
                                       int frame, bool is_ghost) {
    static const int head_offset_x[] = {0, 0, 0, 0};
    static const int head_offset_y[] = {-22, -22, -22, -22};
    static const int head_dir_to_row[] = {2, 0, 3, 1};

    uint8_t alpha = is_ghost ? 128 : 255;
    SDL_SetTextureAlphaMod(body_tex, alpha);
    SDL_SetTextureAlphaMod(head_tex, alpha);

    if (direction < 0 || direction > 3)
        direction = 0;

    int total_frames = (direction < 2) ? 6 : 5;
    int frame_clamped = frame % total_frames;

    renderer->draw_frame(body_tex, frame_clamped * BODY_W, direction * BODY_H, BODY_W, BODY_H,
                         player_position_x, player_position_y);

    int hx = player_position_x + head_offset_x[direction];
    int hy = player_position_y + head_offset_y[direction];
    renderer->draw_frame(head_tex, 0, head_dir_to_row[direction] * HEAD_H, HEAD_W, HEAD_H, hx, hy);

    SDL_SetTextureAlphaMod(body_tex, 255);
    SDL_SetTextureAlphaMod(head_tex, 255);
}

void CharacterRenderer::draw_equipped_item(int player_position_x, int player_position_y,
                                           SDL_Texture* itex, ItemType itype, int tile_w) {
    switch (itype) {
        case ItemType::WEAPON:
        case ItemType::STAFF:
            renderer->draw_frame(itex, 0, 0, ITEM_SPRITE_SIZE, ITEM_SPRITE_SIZE,
                                 player_position_x + tile_w / 2 + 2, player_position_y);
            break;
        case ItemType::SHIELD:
            renderer->draw_frame(itex, 0, 0, ITEM_SPRITE_SIZE, ITEM_SPRITE_SIZE,
                                 player_position_x - tile_w / 2 - 2 - ITEM_SPRITE_SIZE,
                                 player_position_y);
            break;
        case ItemType::HELMET: {
            SDL_Rect src = {0, 0, ITEM_SPRITE_SIZE, ITEM_SPRITE_SIZE};
            SDL_Rect dst = {player_position_x + (BODY_W - HELMET_DRAW_SIZE) / 2,
                            player_position_y - 12, HELMET_DRAW_SIZE, HELMET_DRAW_SIZE};
            SDL_RenderCopy(renderer->get_sdl_renderer(), itex, &src, &dst);
            break;
        }
        default:
            return;
    }
}

void CharacterRenderer::draw_shadow(int center_x, int foot_y, int width) {
    renderer->draw_shadow(center_x, foot_y, width);
}

void CharacterRenderer::draw_nickname(const std::string& nickname, int px, int py,
                                      SDL_Color color) {
    int nick_center_x = px + BODY_W / 2;
    int nick_y = py + BODY_H + 3;
    SDL_Color shadow = {0, 0, 0, 200};
    renderer->draw_label(nickname, nick_center_x + 1, nick_y + 1, shadow);
    renderer->draw_label(nickname, nick_center_x, nick_y, color);
}
