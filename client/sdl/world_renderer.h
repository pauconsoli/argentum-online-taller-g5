#ifndef WORLD_RENDERER_H
#define WORLD_RENDERER_H

#include <string>

#include "camera.h"
#include "clan_panel.h"
#include "help_menu.h"
#include "hud.h"
#include "mini_chat.h"
#include "npc_renderer.h"
#include "player_renderer.h"
#include "renderer.h"
#include "sdl_config.h"
#include "sprite_manager.h"
#include "terrain_renderer.h"

class ClientMap;
class GameState;

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

 public:
    WorldRenderer(Renderer&, SpriteManager&, TerrainRenderer&, NPCRenderer&, PlayerRenderer&, Hud&,
                  MiniChat&, HelpMenu&, ClanPanel&, Camera&, const SdlConfig&);

    void draw(const GameState& state, const ClientMap& map, int direction, int current_frame,
              bool chat_active, const std::string& chat_input, bool music_paused, int screen_w,
              int screen_h);
};

#endif
