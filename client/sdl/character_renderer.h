#ifndef CHARACTER_RENDERER_H
#define CHARACTER_RENDERER_H

#include <string>

#include <SDL2/SDL.h>

#include "renderer.h"

enum class ItemType { WEAPON, STAFF, ARMOR, HELMET, SHIELD, OTHER };

class CharacterRenderer {
 private:
    static constexpr int BODY_W = 27;
    static constexpr int BODY_H = 48;
    static constexpr int HEAD_W = 27;
    static constexpr int HEAD_H = 64;
    static constexpr int ITEM_SPRITE_SIZE = 32;
    static constexpr int HELMET_DRAW_SIZE = 24;

    Renderer* renderer;

 public:
    explicit CharacterRenderer(Renderer* renderer);

    void draw_character(SDL_Texture* body_tex, SDL_Texture* head_tex, int px, int py, int direction,
                        int frame, bool is_ghost);
    void draw_equipped_item(int px, int py, SDL_Texture* itex, ItemType itype, int tile_w);
    void draw_nickname(const std::string& nickname, int px, int py);
};

#endif
