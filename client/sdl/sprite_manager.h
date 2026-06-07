#ifndef SPRITE_MANAGER_H
#define SPRITE_MANAGER_H

#include <map>
#include <string>

#include <SDL2/SDL.h>
#include <SDL_image.h>

#include "common/world/terrain_type.h"

class SpriteManager {
 private:
    SDL_Renderer* sdl_renderer;
    std::map<std::string, SDL_Texture*> textures;

    static const char* terrain_key(TerrainType t);

 public:
    explicit SpriteManager(SDL_Renderer* renderer);
    ~SpriteManager();

    SpriteManager(const SpriteManager&) = delete;
    SpriteManager& operator=(const SpriteManager&) = delete;

    void load(const std::string& id, const std::string& path);
    SDL_Texture* get(const std::string& id) const;

    void load_terrain_textures(const std::string& assets_dir);
    SDL_Texture* get_terrain(TerrainType t) const;
};

#endif
