#include "sprite_manager.h"
#include <stdexcept>

SpriteManager::SpriteManager(SDL_Renderer* renderer) : sdl_renderer(renderer) {}

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
