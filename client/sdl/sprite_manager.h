#ifndef SPRITE_MANAGER_H
#define SPRITE_MANAGER_H

#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>

#include <SDL2/SDL.h>
#include <SDL_image.h>

#include "common/world/terrain_type.h"
#include "npc_visual_type.h"

// Descripción de un recorte estático en Recursos/Graficos/<file_num>.png.
// Solo se usan entradas con frames==1 de graficos.ini (imágenes fijas, no animadas).
struct GrhEntry {
    int file_num;  // número del PNG en Recursos/Graficos/
    int x, y;      // esquina superior izquierda del recorte dentro del PNG
    int w, h;      // tamaño del recorte en píxeles (casi siempre 32×32)
};

class SpriteManager {
 private:
    SDL_Renderer* sdl_renderer;
    std::map<std::string, SDL_Texture*> textures;
    std::unordered_map<int, GrhEntry> grh_index;       // GrhId → descripción del recorte
    std::unordered_map<int, SDL_Texture*> grh_sheets;  // fileNum → textura del spritesheet
    std::string assets_dir;
    std::string recursos_dir;  // path a Recursos/ (spritesheets del AO original)

    static const char* terrain_key(TerrainType t);
    // Devuelve nullptr si el terreno no tiene overlay.
    static const char* terrain_overlay_key(TerrainType t);
    static std::string body_key(uint8_t race, uint8_t klass);
    static std::string head_key(uint16_t head_index);

    SDL_Texture* load_lazy(const std::string& key, const std::string& path);
    // Igual que load_lazy pero cachea nullptr al primer intento fallido.
    // Usar solo para assets opcionales (overlays de transición).
    SDL_Texture* load_optional_lazy(const std::string& key, const std::string& path);

 public:
    explicit SpriteManager(SDL_Renderer* renderer);
    ~SpriteManager();

    SpriteManager(const SpriteManager&) = delete;
    SpriteManager& operator=(const SpriteManager&) = delete;

    void load(const std::string& id, const std::string& path);
    SDL_Texture* get(const std::string& id) const;

    void load_terrain_textures(const std::string& assets_dir);
    SDL_Texture* get_terrain(TerrainType t) const;
    // Retorna nullptr si el terreno no tiene overlay o el PNG todavía no existe.
    SDL_Texture* get_terrain_overlay(TerrainType t);
    // Retorna nullptr si el PNG todavía no existe. La clave es el nombre del archivo sin extensión,
    // e.g. "terrain_overlay_water_sand_north". Resultado cacheado: el intento fallido no
    // reintenta IMG_Load en frames siguientes.
    SDL_Texture* get_transition_overlay(const char* key);
    SDL_Texture* get_tree() const;

    void load_body_textures(const std::string& assets_dir);
    SDL_Texture* get_body(uint8_t race, uint8_t klass) const;

    // Head sprites: 27x256 strip, 4 directions (south/north/west/east), 64px each.
    // dir 0=south, 1=north, 2=west, 3=east -> src_rect.y = dir * 64
    SDL_Texture* get_head(uint16_t head_index);

    // Item icons: 32x32 RGBA. key es la clave de textura (e.g. "item_espada", "item_pocion_vida").
    SDL_Texture* get_item(const std::string& key);

    // Sprite de monedas de oro (item_oro.png, 32x32).
    SDL_Texture* get_gold();

    // NPC sprites: 4 filas (sur/norte/oeste/este), N frames por fila.
    // Carga lazy desde assets/sprites/npcs/npc_<tipo>.png.
    SDL_Texture* get_npc(NPCVisualType type);

    // Mapea el nombre de item del protocolo a la clave de textura correspondiente.
    // Fallback: "item_espada" si el nombre no está en la tabla.
    static std::string item_key_for_name(const std::string& name);

    // Lee Recursos/init/graficos.ini y llena grh_index (solo entradas frames==1).
    void load_grh_index(const std::string& recursos_dir);
    // Carga lazy Recursos/Graficos/<file_num>.png con color-key blanco.
    // Cachea nullptr si el archivo no existe (no reintenta en frames siguientes).
    SDL_Texture* get_grh_sheet(int file_num);
};

#endif
