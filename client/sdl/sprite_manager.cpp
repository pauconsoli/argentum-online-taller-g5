#include "sprite_manager.h"

#include <stdexcept>

SpriteManager::SpriteManager(SDL_Renderer* renderer): sdl_renderer(renderer) {}

SpriteManager::~SpriteManager() {
    for (auto& pair : textures) {
        SDL_DestroyTexture(pair.second);
    }
}

void SpriteManager::load(const std::string& id, const std::string& path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (surface == nullptr) {
        throw std::runtime_error(IMG_GetError());
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(sdl_renderer, surface);
    SDL_FreeSurface(surface);
    if (texture == nullptr) {
        throw std::runtime_error(SDL_GetError());
    }
    textures[id] = texture;
}

SDL_Texture* SpriteManager::get(const std::string& id) const {
    auto it = textures.find(id);
    if (it == textures.end()) {
        throw std::runtime_error("Sprite no encontrado: " + id);
    }
    return it->second;
}

const char* SpriteManager::terrain_key(TerrainType t) {
    switch (t) {
        case TerrainType::GRASS:
            return "terrain_grass";
        case TerrainType::WATER:
            return "terrain_water";
        case TerrainType::DIRT:
            return "terrain_dirt";
        case TerrainType::STONE:
            return "terrain_stone";
        case TerrainType::SAND:
            return "terrain_sand";
    }
    return "terrain_grass";
}

void SpriteManager::load_terrain_textures(const std::string& assets_dir) {
    const std::string sep = assets_dir.back() == '/' ? "" : "/";
    load("terrain_grass", assets_dir + sep + "grass_tile.png");
    load("terrain_water", assets_dir + sep + "water.png");
    load("terrain_dirt", assets_dir + sep + "dirt.png");
    load("terrain_stone", assets_dir + sep + "stone.png");
    load("terrain_sand", assets_dir + sep + "sand.png");
}

SDL_Texture* SpriteManager::get_terrain(TerrainType t) const {
    return get(terrain_key(t));
}
