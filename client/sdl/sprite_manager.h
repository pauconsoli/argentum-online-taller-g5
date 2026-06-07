#ifndef SPRITE_MANAGER_H
#define SPRITE_MANAGER_H

#include <cstdint>
#include <map>
#include <string>

#include <SDL2/SDL.h>
#include <SDL_image.h>

#include "common/world/terrain_type.h"

class SpriteManager {
 private:
    SDL_Renderer* sdl_renderer;
    std::map<std::string, SDL_Texture*> textures;
    std::string assets_dir;

    static const char* terrain_key(TerrainType t);
    static std::string body_key(uint8_t race, uint8_t klass);
    static std::string head_key(uint16_t head_index);
    static std::string item_key(uint16_t item_id);

    SDL_Texture* load_lazy(const std::string& key, const std::string& path);

 public:
    explicit SpriteManager(SDL_Renderer* renderer);
    ~SpriteManager();

    SpriteManager(const SpriteManager&) = delete;
    SpriteManager& operator=(const SpriteManager&) = delete;

    void load(const std::string& id, const std::string& path);
    SDL_Texture* get(const std::string& id) const;

    void load_terrain_textures(const std::string& assets_dir);
    SDL_Texture* get_terrain(TerrainType t) const;

    void load_body_textures(const std::string& assets_dir);
    SDL_Texture* get_body(uint8_t race, uint8_t klass) const;

    // Head sprites: 27x256 strip, 4 directions (south/north/west/east), 64px each.
    // dir 0=south, 1=north, 2=west, 3=east -> src_rect.y = dir * 64
    SDL_Texture* get_head(uint16_t head_index);

    // Item icons: 32x32 RGBA.
    SDL_Texture* get_item(uint16_t item_id);
};

#endif
