#ifndef WORLD_RENDERER_H
#define WORLD_RENDERER_H

#include <cstdint>
#include <string>
#include <vector>

#include "../assets/sprite_manager.h"
#include "../core/camera.h"
#include "../core/sdl_config.h"
#include "../ui/clan_panel.h"
#include "../ui/help_menu.h"
#include "../ui/hud.h"
#include "../ui/mini_chat.h"
#include "npc_renderer.h"
#include "player_renderer.h"
#include "renderer.h"
#include "terrain_renderer.h"

class ClientMap;
class GameState;
struct GroundItemSnapshot;
struct PlayerSnapshot;
struct NPCSnapshot;

enum class FringeType : uint8_t { TREE, ITEM, PLAYER, NPC };

struct FringeEntry {
    int y_row;
    FringeType type;
    int col;      // TREE: tile column (row == y_row)
    uint32_t id;  // PLAYER: pid, NPC: nid
    union {
        const GroundItemSnapshot* item;
        const PlayerSnapshot* ps;
        const NPCSnapshot* ns;
    };

    static FringeEntry make_tree(int col, int row);
    static FringeEntry make_item(const GroundItemSnapshot& gi);
    static FringeEntry make_player(uint32_t pid, const PlayerSnapshot& p);
    static FringeEntry make_npc(uint32_t nid, const NPCSnapshot& n);
};

struct TileBounds {
    int start_col, end_col, start_row, end_row;
};

class WorldRenderer {
    Renderer& renderer;
    SpriteManager& sprite_manager;
    TerrainRenderer& terrain_renderer;
    NPCRenderer& npc_renderer;
    PlayerRenderer& player_renderer;
    Hud& hud;
    MiniChat& mini_chat;
    HelpMenu& help_menu;
    ClanPanel& clan_panel;
    Camera& camera;
    const SdlConfig& config;

    // Sprite por defecto cuando no se encuentra la textura de un item en el suelo.
    static constexpr const char* FALLBACK_ITEM_SPRITE = "item_espada";

    TileBounds draw_terrain(const ClientMap& map, int screen_w, int screen_h);
    static std::vector<FringeEntry> build_fringe(const TileBounds& bounds, const GameState& state,
                                                 const ClientMap& map);
    void draw_fringe(const std::vector<FringeEntry>& fringe, const GameState& state, int direction,
                     int current_frame);
    void draw_ground_item(const GroundItemSnapshot& gi);
    void draw_ui(const GameState& state, bool chat_active, const std::string& chat_input,
                 bool music_paused, int screen_h);

 public:
    WorldRenderer(Renderer&, SpriteManager&, TerrainRenderer&, NPCRenderer&, PlayerRenderer&, Hud&,
                  MiniChat&, HelpMenu&, ClanPanel&, Camera&, const SdlConfig&);

    void draw(const GameState& state, const ClientMap& map, int direction, int current_frame,
              bool chat_active, const std::string& chat_input, bool music_paused, int screen_w,
              int screen_h);
};

#endif
