#include "character_renderer.h"

CharacterRenderer::CharacterRenderer(Renderer* renderer): renderer(renderer) {}

void CharacterRenderer::draw_character(SDL_Texture* body_texture, SDL_Texture* head_texture,
                                       int player_position_x, int player_position_y, int direction,
                                       int frame, bool is_ghost) {

    static const int head_position_x[] = {0, 0, 0, 0};
    static const int head_position_y[] = {-22, -22, -22, -22};
    static const int head_direction[] = {2, 0, 3, 1};

    // Fantasma, mitad de opacidad
    uint8_t alpha = is_ghost ? 128 : 255;
    SDL_SetTextureAlphaMod(body_texture, alpha);
    SDL_SetTextureAlphaMod(head_texture, alpha);

    if (direction < 0 || direction > 3)
        direction = 0;

    int total_frames = (direction < 2) ? 6 : 5;
    int limited_frame = frame % total_frames;

    renderer->draw_frame(body_texture, limited_frame * BODY_W, direction * BODY_H, BODY_W, BODY_H,
                         player_position_x, player_position_y);

    int head_x = player_position_x + head_position_x[direction];
    int head_y = player_position_y + head_position_y[direction];

    renderer->draw_frame(head_texture, 0, head_direction[direction] * HEAD_H, HEAD_W, HEAD_H,
                         head_x, head_y);

    // Restaura opacidad total para no contaminar renders posteriores.
    SDL_SetTextureAlphaMod(body_texture, 255);
    SDL_SetTextureAlphaMod(head_texture, 255);
}

void CharacterRenderer::draw_equipped_item(int player_position_x, int player_position_y,
                                           SDL_Texture* item_texture, ItemType item_type,
                                           int tile_w) {
    switch (item_type) {
        case ItemType::WEAPON:
        case ItemType::STAFF:
            renderer->draw_frame(item_texture, 0, 0, ITEM_SPRITE_SIZE, ITEM_SPRITE_SIZE,
                                 player_position_x + tile_w / 2 + 2, player_position_y);
            break;
        case ItemType::SHIELD:
            renderer->draw_frame(item_texture, 0, 0, ITEM_SPRITE_SIZE, ITEM_SPRITE_SIZE,
                                 player_position_x - ITEM_SPRITE_SIZE + 14, player_position_y);
            break;
        case ItemType::HELMET: {
            SDL_Rect src = {0, 0, ITEM_SPRITE_SIZE, ITEM_SPRITE_SIZE};
            SDL_Rect dst = {player_position_x + (BODY_W - HELMET_DRAW_SIZE) / 2,
                            player_position_y - 20, HELMET_DRAW_SIZE, HELMET_DRAW_SIZE};
            // 20px -> cuanto lo subimos
            SDL_RenderCopy(renderer->get_sdl_renderer(), item_texture, &src, &dst);
            break;
        }
        case ItemType::ARMOR: {
            renderer->draw_frame(item_texture, 0, 0, BODY_W, BODY_H, player_position_x,
                                 player_position_y);
            break;
        }
        default:
            return;
    }
}
void CharacterRenderer::draw_shadow(int center_x, int foot_y, int width) {
    renderer->draw_shadow(center_x, foot_y, width);
}

void CharacterRenderer::draw_nickname(const std::string& nickname, int position_x, int position_y,
                                      SDL_Color color) {
    int nick_center_x = position_x + BODY_W / 2;
    int nick_y = position_y + BODY_H + 3;
    SDL_Color shadow = {0, 0, 0, 200};
    renderer->draw_label(nickname, nick_center_x + 1, nick_y + 1, shadow);
    renderer->draw_label(nickname, nick_center_x, nick_y, color);
}
