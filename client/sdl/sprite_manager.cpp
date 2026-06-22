#include "sprite_manager.h"

#include <cstdio>
#include <fstream>
#include <stdexcept>
#include <unordered_map>

SpriteManager::SpriteManager(SDL_Renderer* renderer): sdl_renderer(renderer), assets_dir("") {}

SpriteManager::~SpriteManager() {
    for (auto& pair : textures) {
        SDL_DestroyTexture(pair.second);
    }
    for (auto& pair : grh_sheets) {
        if (pair.second)
            SDL_DestroyTexture(pair.second);
    }
}

void SpriteManager::load(const std::string& id, const std::string& path) {
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (surface == nullptr) {
        throw std::runtime_error(IMG_GetError());
    }
    // PNGs sin canal alpha (fondo blanco opaco): hacer transparente el blanco.
    if (surface->format->Amask == 0) {
        Uint32 white = SDL_MapRGB(surface->format, 255, 255, 255);
        SDL_SetColorKey(surface, SDL_TRUE, white);
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
            return "terrain_city_floor_diagonal";
        case TerrainType::CITY_WALL_1:
            return "terrain_city_wall_stone";
        case TerrainType::CITY_FLOOR_2:
            return "terrain_city_floor_cobble";
        case TerrainType::CITY_WALL_2:
            return "terrain_city_floor_cobble";
    }
    return "terrain_grass";
}

void SpriteManager::load_terrain_textures(const std::string& dir) {
    this->assets_dir = dir;
    const std::string sep = assets_dir.back() == '/' ? "" : "/";
    const std::string terrain_dir = assets_dir + sep + "sprites/terrain/";
    load("terrain_grass", terrain_dir + "grass_tile.png");
    load("terrain_water", terrain_dir + "ao_water.png");
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
    load("terrain_city_floor_cobble", terrain_dir + "city_floor_cobble.png");
    load("terrain_city_floor_diagonal", terrain_dir + "city_floor_diagonal.png");
    load("terrain_city_wall_stone", terrain_dir + "city_wall_stone.png");
    load("tree", terrain_dir + "tree.png");
    // Overlays de terreno con clave fija (opcionales: el PNG puede no existir todavía).
    try_load_cached("terrain_overlay_dungeon_entrance_1",
                    terrain_dir + "terrain_overlay_dungeon_entrance_1.png");
    try_load_cached("terrain_overlay_dungeon_entrance_2",
                    terrain_dir + "terrain_overlay_dungeon_entrance_2.png");
    try_load_cached("terrain_overlay_city_wall_2", terrain_dir + "terrain_overlay_city_wall_2.png");
    // Todos los items con clave de nombre conocida (precarga eager completa).
    const std::string items_dir = assets_dir + sep + "sprites/items/";
    try_load_cached("item_pocion_vida", items_dir + "item_pocion_vida.png");
    try_load_cached("item_pocion_mana", items_dir + "item_pocion_mana.png");
    try_load_cached("item_flauta_elfica", items_dir + "item_flauta_elfica.png");
    try_load_cached("item_oro", items_dir + "item_oro.png");
    try_load_cached("item_espada", items_dir + "item_espada.png");
    try_load_cached("item_hacha", items_dir + "item_hacha.png");
    try_load_cached("item_martillo", items_dir + "item_martillo.png");
    try_load_cached("item_vara_fresno", items_dir + "item_vara_fresno.png");
    try_load_cached("item_baculo_nudoso", items_dir + "item_baculo_nudoso.png");
    try_load_cached("item_baculo_engarzado", items_dir + "item_baculo_engarzado.png");
    try_load_cached("item_arco_simple", items_dir + "item_arco_simple.png");
    try_load_cached("item_arco_compuesto", items_dir + "item_arco_compuesto.png");
    try_load_cached("item_armadura_cuero", items_dir + "item_armadura_cuero.png");
    try_load_cached("item_armadura_placas", items_dir + "item_armadura_placas.png");
    try_load_cached("item_tunica_azul", items_dir + "item_tunica_azul.png");
    try_load_cached("item_capucha", items_dir + "item_capucha.png");
    try_load_cached("item_casco_hierro", items_dir + "item_casco_hierro.png");
    try_load_cached("item_escudo_tortuga", items_dir + "item_escudo_tortuga.png");
    try_load_cached("item_escudo_hierro", items_dir + "item_escudo_hierro.png");
    try_load_cached("item_sombrero_magico", items_dir + "item_sombrero_magico.png");
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
    return try_load(key, path);  // nullptr si el archivo todavía no existe
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

SDL_Texture* SpriteManager::try_load(const std::string& key, const std::string& path) {
    auto it = textures.find(key);
    if (it != textures.end()) {
        return it->second;
    }
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (surface == nullptr) {
        return nullptr;
    }
    // PNGs sin canal alpha (fondo blanco opaco): hacer transparente el blanco.
    if (surface->format->Amask == 0) {
        Uint32 white = SDL_MapRGB(surface->format, 255, 255, 255);
        SDL_SetColorKey(surface, SDL_TRUE, white);
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(sdl_renderer, surface);
    SDL_FreeSurface(surface);
    if (texture == nullptr) {
        return nullptr;
    }
    textures[key] = texture;
    return texture;
}

SDL_Texture* SpriteManager::try_load_cached(const std::string& key, const std::string& path) {
    auto it = textures.find(key);
    if (it != textures.end()) {
        return it->second;  // puede ser nullptr si se intentó y el PNG no existía
    }
    SDL_Texture* tex = try_load(key, path);
    if (!tex) {
        textures[key] = nullptr;  // cachear la ausencia; los overlays opcionales no se reintentarán
    }
    return tex;
}

SDL_Texture* SpriteManager::get_head(uint16_t head_index) {
    if (head_index == 0)
        head_index = 1;
    std::string key = head_key(head_index);
    // Busca en cache; debería estar precargado por load_head_textures.
    auto it = textures.find(key);
    if (it != textures.end())
        return it->second;
    // Fallback: cabeza no precargada → devolver head_1 si está disponible.
    auto it2 = textures.find(head_key(1));
    return (it2 != textures.end()) ? it2->second : nullptr;
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
        {"Poción de vida", "item_pocion_vida"},
        {"Poción de maná", "item_pocion_mana"},
    };
    auto it = table.find(name);
    return it != table.end() ? it->second : "item_espada";
}

SDL_Texture* SpriteManager::get_item(const std::string& key) {
    // Busca en cache; debería estar precargado por load_terrain_textures.
    auto it = textures.find(key);
    if (it != textures.end())
        return it->second;
    // Fallback: item no precargado (e.g. key desconocida), intento tolerante con cacheo de fallo.
    const std::string sep = assets_dir.empty() || assets_dir.back() == '/' ? "" : "/";
    return try_load_cached(key, assets_dir + sep + "sprites/items/" + key + ".png");
}

SDL_Texture* SpriteManager::get_gold() {
    // Precargado en load_terrain_textures; búsqueda directa en cache.
    auto it = textures.find("item_oro");
    return (it != textures.end()) ? it->second : nullptr;
}

SDL_Texture* SpriteManager::get_transition_overlay(const char* key) {
    auto it = textures.find(key);
    if (it != textures.end())
        return it->second;
    const std::string sep = assets_dir.empty() || assets_dir.back() == '/' ? "" : "/";
    std::string path = assets_dir + sep + "sprites/terrain/" + key + ".png";
    return try_load_cached(key, path);
}

void SpriteManager::load_grh_index(const std::string& res_dir) {
    recursos_dir = res_dir;
    const std::string sep = res_dir.empty() || res_dir.back() == '/' ? "" : "/";
    std::string path = res_dir + sep + "init/graficos.ini";
    std::ifstream file(path);
    if (!file.is_open()) {
        SDL_Log("load_grh_index: no se pudo abrir %s", path.c_str());
        return;  // fallo no fatal: el juego sigue con el sistema de tiles sintéticos
    }
    std::string line;
    while (std::getline(file, line)) {
        // graficos.ini usa CRLF (\r\n); descartamos el \r sobrante
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        // Saltear vacías, encabezados [INIT]/[Graphics], y claves que no son Grh
        if (line.size() < 4 || line[0] != 'G' || line[1] != 'r' || line[2] != 'h')
            continue;
        // Formato: GrhN=frames-fileNum-x-y-w-h
        int grh_id, frames, fn, x, y, w, h;
        if (std::sscanf(line.c_str(), "Grh%d=%d-%d-%d-%d-%d-%d", &grh_id, &frames, &fn, &x, &y, &w,
                        &h) != 7)
            continue;
        if (frames != 1)
            continue;  // imágenes animadas se ignoran por ahora
        grh_index[grh_id] = {fn, x, y, w, h};
    }
}

SDL_Texture* SpriteManager::get_grh_sheet(int file_num) {
    auto it = grh_sheets.find(file_num);
    if (it != grh_sheets.end())
        return it->second;  // puede ser nullptr si el PNG no existe
    const std::string sep = recursos_dir.empty() || recursos_dir.back() == '/' ? "" : "/";
    std::string path = recursos_dir + sep + "Graficos/" + std::to_string(file_num) + ".png";
    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        grh_sheets[file_num] = nullptr;  // cachear el intento fallido
        return nullptr;
    }
    // PNGs de AO sin canal alpha usan fondo blanco como color de transparencia
    if (surface->format->Amask == 0) {
        Uint32 white = SDL_MapRGB(surface->format, 255, 255, 255);
        SDL_SetColorKey(surface, SDL_TRUE, white);
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(sdl_renderer, surface);
    SDL_FreeSurface(surface);
    grh_sheets[file_num] = tex;
    return tex;
}

void SpriteManager::load_head_textures() {
    const std::string sep = assets_dir.empty() || assets_dir.back() == '/' ? "" : "/";
    const std::string heads_dir = assets_dir + sep + "sprites/characters/heads/";
    // Solo las cabezas realmente usadas: una por raza de jugador y las dos de NPCs con cabeza.
    static const uint16_t used_heads[] = {1, 3, 30, 101, 300, 400};
    for (uint16_t idx : used_heads) {
        std::string key = head_key(idx);
        try_load_cached(key, heads_dir + key + ".png");
    }
}

void SpriteManager::load_npc_textures() {
    const std::string sep = assets_dir.empty() || assets_dir.back() == '/' ? "" : "/";
    const std::string npcs_dir = assets_dir + sep + "sprites/npcs/";
    // Debe coincidir exactamente con los filenames del switch de get_npc.
    static const char* npc_files[] = {
        "npc_banker.png",    "npc_priest.png",      "npc_merchant.png",       "npc_goblin.png",
        "npc_skeleton.png",  "npc_zombie.png",      "npc_spider.png",         "npc_orc.png",
        "npc_golem_ice.png", "npc_golem_stone.png", "npc_golem_infernal.png",
    };
    for (const char* filename : npc_files) {
        std::string key = std::string("npc_") + filename;
        try_load_cached(key, npcs_dir + filename);
    }
}

void SpriteManager::load_grh_sheets() {
    // Precarga todos los spritesheets únicos del índice.
    // Es un no-op si load_grh_index no fue llamado o si Recursos/Graficos no existe.
    for (auto& [grh_id, entry] : grh_index) {
        get_grh_sheet(entry.file_num);  // cachea el resultado en grh_sheets
    }
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
    // Busca en cache; debería estar precargado por load_npc_textures.
    auto it = textures.find(key);
    return (it != textures.end()) ? it->second : nullptr;
}
