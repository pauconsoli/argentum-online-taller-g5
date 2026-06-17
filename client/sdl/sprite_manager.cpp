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
        case TerrainType::WOOD:
            return "terrain_wood";
        case TerrainType::DUNGEON_FLOOR_1:
        case TerrainType::DUNGEON_ENTRANCE_1:
            return "terrain_dungeon_floor";
        case TerrainType::DUNGEON_FLOOR_2:
        case TerrainType::DUNGEON_ENTRANCE_2:
            return "terrain_dungeon_floor_2";
        case TerrainType::DUNGEON_WALL_1:
            return "terrain_dungeon_wall";
        case TerrainType::DUNGEON_WALL_2:
            return "terrain_dungeon_wall_2";
        case TerrainType::CITY_FLOOR_1:
        case TerrainType::CITY_WALL_1:
            return "terrain_city_floor_1";
        case TerrainType::CITY_FLOOR_2:
        case TerrainType::CITY_WALL_2:
            return "terrain_city_floor_2";
    }
    return "terrain_grass";
}

void SpriteManager::load_terrain_textures(const std::string& dir) {
    this->assets_dir = dir;
    const std::string sep = assets_dir.back() == '/' ? "" : "/";
    const std::string terrain_dir = assets_dir + sep + "sprites/terrain/";
    load("terrain_grass", terrain_dir + "grass_tile.png");
    load("terrain_water", terrain_dir + "water.png");
    load("terrain_dirt", terrain_dir + "dirt.png");
    load("terrain_stone", terrain_dir + "stone.png");
    load("terrain_sand", terrain_dir + "sand.png");
    load("terrain_dungeon_floor", terrain_dir + "dungeon_floor.png");
    load("terrain_dungeon_wall", terrain_dir + "dungeon_wall.png");
    load("terrain_dungeon_floor_2", terrain_dir + "dungeon_floor_2.png");
    load("terrain_dungeon_wall_2", terrain_dir + "dungeon_wall_2.png");
    load("terrain_wood", terrain_dir + "wood.png");
    load("terrain_city_floor_1", terrain_dir + "city_floor_1.png");
    load("terrain_city_floor_2", terrain_dir + "city_floor_2.png");
    load("tree", terrain_dir + "tree.png");
    // Pociones, flauta y oro cargadas eagerly: tienen clave de nombre en lugar de numérica.
    const std::string items_dir = assets_dir + sep + "sprites/items/";
    load_lazy("item_pocion_vida", items_dir + "item_pocion_vida.png");
    load_lazy("item_pocion_mana", items_dir + "item_pocion_mana.png");
    load_lazy("item_flauta_elfica", items_dir + "item_flauta_elfica.png");
    load_lazy("item_oro", items_dir + "item_oro.png");
}

SDL_Texture* SpriteManager::get_terrain(TerrainType t) const {
    return get(terrain_key(t));
}

const char* SpriteManager::terrain_overlay_key(TerrainType t) {
    switch (t) {
        case TerrainType::DUNGEON_ENTRANCE_1:
            return "terrain_overlay_dungeon_entrance_1";
        case TerrainType::DUNGEON_ENTRANCE_2:
            return "terrain_overlay_dungeon_entrance_2";
        case TerrainType::CITY_WALL_1:
            return "terrain_overlay_city_wall_1";
        case TerrainType::CITY_WALL_2:
            return "terrain_overlay_city_wall_2";
        default:
            return nullptr;
    }
}

SDL_Texture* SpriteManager::get_terrain_overlay(TerrainType t) {
    const char* key = terrain_overlay_key(t);
    if (!key)
        return nullptr;
    // Archivo esperado: assets/sprites/terrain/<key>.png (e.g. terrain_overlay_city_wall_1.png)
    const std::string sep = assets_dir.empty() || assets_dir.back() == '/' ? "" : "/";
    std::string path = assets_dir + sep + "sprites/terrain/" + key + ".png";
    return load_lazy(key, path);  // nullptr si el archivo todavía no existe
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
    const std::string bodies_dir = dir + sep + "sprites/characters/bodies/";
    for (uint8_t race = 0; race < 4; ++race) {
        for (uint8_t klass = 0; klass < 4; ++klass) {
            std::string key = body_key(race, klass);
            std::string path = bodies_dir + key + ".png";
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
    std::string path = assets_dir + sep + "sprites/characters/heads/" + key + ".png";
    SDL_Texture* tex = load_lazy(key, path);
    if (tex == nullptr) {
        tex = load_lazy(head_key(1), assets_dir + sep + "sprites/characters/heads/head_1.png");
    }
    return tex;
}

std::string SpriteManager::item_key_for_name(const std::string& name) {
    static const std::unordered_map<std::string, std::string> table = {
        {"Espada", "item_espada"},
        {"Hacha", "item_hacha"},
        {"Martillo", "item_martillo"},
        {"Vara de fresno", "item_vara_fresno"},
        {"Báculo nudoso", "item_baculo_nudoso"},
        {"Báculo engarzado", "item_baculo_engarzado"},
        {"Arco simple", "item_arco_simple"},
        {"Arco compuesto", "item_arco_compuesto"},
        {"Armadura de cuero", "item_armadura_cuero"},
        {"Armadura de placas", "item_armadura_placas"},
        {"Túnica azul", "item_tunica_azul"},
        {"Capucha", "item_capucha"},
        {"Casco de hierro", "item_casco_hierro"},
        {"Escudo de tortuga", "item_escudo_tortuga"},
        {"Escudo de hierro", "item_escudo_hierro"},
        {"Sombrero mágico", "item_sombrero_magico"},
        {"Flauta élfica", "item_flauta_elfica"},
        {"Pocion de vida", "item_pocion_vida"},
        {"Pocion de mana", "item_pocion_mana"},
    };
    auto it = table.find(name);
    return it != table.end() ? it->second : "item_espada";
}

SDL_Texture* SpriteManager::get_item(const std::string& key) {
    const std::string sep = assets_dir.empty() || assets_dir.back() == '/' ? "" : "/";
    std::string path = assets_dir + sep + "sprites/items/" + key + ".png";
    return load_lazy(key, path);
}

SDL_Texture* SpriteManager::get_gold() {
    const std::string sep = assets_dir.empty() || assets_dir.back() == '/' ? "" : "/";
    return load_lazy("item_oro", assets_dir + sep + "sprites/items/item_oro.png");
}

SDL_Texture* SpriteManager::get_npc(NPCVisualType type) {
    const char* filename = nullptr;
    switch (type) {
        case NPCVisualType::BANKER:
            filename = "npc_banker.png";
            break;
        case NPCVisualType::PRIEST:
            filename = "npc_priest.png";
            break;
        case NPCVisualType::MERCHANT:
            filename = "npc_merchant.png";
            break;
        case NPCVisualType::GOBLIN:
            filename = "npc_goblin.png";
            break;
        case NPCVisualType::SKELETON:
            filename = "npc_skeleton.png";
            break;
        case NPCVisualType::ZOMBIE:
            filename = "npc_zombie.png";
            break;
        case NPCVisualType::SPIDER:
            filename = "npc_spider.png";
            break;
        case NPCVisualType::ORC:
            filename = "npc_orc.png";
            break;
        case NPCVisualType::GOLEM_ICE:
            filename = "npc_golem_ice.png";
            break;
        case NPCVisualType::GOLEM_STONE:
            filename = "npc_golem_stone.png";
            break;
        case NPCVisualType::GOLEM_INFERNAL:
            filename = "npc_golem_infernal.png";
            break;
        default:
            return nullptr;
    }
    std::string key = std::string("npc_") + filename;
    const std::string sep = assets_dir.empty() || assets_dir.back() == '/' ? "" : "/";
    std::string path = assets_dir + sep + "sprites/npcs/" + filename;
    return load_lazy(key, path);
}
