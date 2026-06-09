#include "sprite_manager.h"

#include <stdexcept>
#include <unordered_map>

SpriteManager::SpriteManager(SDL_Renderer* renderer): sdl_renderer(renderer), assets_dir("") {}

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
        case TerrainType::DUNGEON_FLOOR:
        case TerrainType::DUNGEON_ENTRANCE:
            return "terrain_dungeon_floor";
        case TerrainType::DUNGEON_WALL:
            return "terrain_dungeon_wall";
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
    load("terrain_dungeon_floor", assets_dir + sep + "dungeon_floor.png");
    load("terrain_dungeon_wall", assets_dir + sep + "dungeon_wall.png");
    load("tree", assets_dir + sep + "tree.png");
    // Pociones y flauta cargadas eagerly: tienen clave de nombre en lugar de numérica.
    const std::string items_dir = assets_dir + sep + "items/";
    load_lazy("item_pocion_vida", items_dir + "item_pocion_vida.png");
    load_lazy("item_pocion_mana", items_dir + "item_pocion_mana.png");
    load_lazy("item_flauta_elfica", items_dir + "item_flauta_elfica.png");
}

SDL_Texture* SpriteManager::get_terrain(TerrainType t) const {
    return get(terrain_key(t));
}

SDL_Texture* SpriteManager::get_tree() const {
    return get("tree");
}

std::string SpriteManager::body_key(uint8_t race, uint8_t klass) {
    return "body_r" + std::to_string(race) + "_c" + std::to_string(klass);
}

void SpriteManager::load_body_textures(const std::string& dir) {
    assets_dir = dir;
    const std::string sep = dir.back() == '/' ? "" : "/";
    for (uint8_t race = 0; race < 4; ++race) {
        for (uint8_t klass = 0; klass < 4; ++klass) {
            std::string key = body_key(race, klass);
            std::string path = dir + sep + key + ".png";
            load(key, path);
        }
    }
}

SDL_Texture* SpriteManager::get_body(uint8_t race, uint8_t klass) const {
    uint8_t safe_race = race < 4 ? race : 0;
    uint8_t safe_klass = klass < 4 ? klass : 0;
    return get(body_key(safe_race, safe_klass));
}

std::string SpriteManager::head_key(uint16_t head_index) {
    return "head_" + std::to_string(head_index);
}

SDL_Texture* SpriteManager::load_lazy(const std::string& key, const std::string& path) {
    auto it = textures.find(key);
    if (it != textures.end()) {
        return it->second;
    }
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (surface == nullptr) {
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(sdl_renderer, surface);
    SDL_FreeSurface(surface);
    if (texture == nullptr) {
        return nullptr;
    }
    textures[key] = texture;
    return texture;
}

SDL_Texture* SpriteManager::get_head(uint16_t head_index) {
    if (head_index == 0)
        head_index = 1;
    std::string key = head_key(head_index);
    const std::string sep = assets_dir.empty() || assets_dir.back() == '/' ? "" : "/";
    std::string path = assets_dir + sep + "heads/" + key + ".png";
    SDL_Texture* tex = load_lazy(key, path);
    if (tex == nullptr) {
        tex = load_lazy(head_key(1), assets_dir + sep + "heads/head_1.png");
    }
    return tex;
}

std::string SpriteManager::item_key_for_name(const std::string& name) {
    static const std::unordered_map<std::string, std::string> table = {
        {"Espada", "item_2"},
        {"Hacha", "item_3"},
        {"Martillo", "item_15"},
        {"Vara de fresno", "item_159"},
        {"Báculo nudoso", "item_401"},
        {"Báculo engarzado", "item_479"},
        {"Arco simple", "item_574"},
        {"Arco compuesto", "item_656"},
        {"Armadura de cuero", "item_30"},
        {"Armadura de placas", "item_1800"},
        {"Túnica azul", "item_1797"},
        {"Capucha", "item_132"},
        {"Casco de hierro", "item_243"},
        {"Escudo de tortuga", "item_38"},
        {"Escudo de hierro", "item_37"},
        {"Sombrero mágico", "item_996"},
        {"Flauta élfica", "item_flauta_elfica"},
        {"Pocion de vida", "item_pocion_vida"},
        {"Pocion de mana", "item_pocion_mana"},
    };
    auto it = table.find(name);
    return it != table.end() ? it->second : "item_2";
}

SDL_Texture* SpriteManager::get_item(const std::string& key) {
    const std::string sep = assets_dir.empty() || assets_dir.back() == '/' ? "" : "/";
    std::string path = assets_dir + sep + "items/" + key + ".png";
    return load_lazy(key, path);
}
