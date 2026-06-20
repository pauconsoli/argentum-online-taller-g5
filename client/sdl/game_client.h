#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

class ClientMap;

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <SDL2/SDL.h>

#include "../client.h"
#include "audio_manager.h"
#include "camera.h"
#include "character_renderer.h"
#include "common/updates/inventory_update.h"
#include "common/updates/snapshot_update.h"
#include "hud.h"
#include "input_handler.h"
#include "mini_chat.h"
#include "renderer.h"
#include "sdl_config.h"
#include "sprite_manager.h"
#include "terrain_renderer.h"

class GameClient {
 private:
    SdlConfig config_;
    SDL_Window* window;
    Renderer* renderer;
    CharacterRenderer* character_renderer;
    Hud* hud;
    MiniChat* mini_chat;
    SpriteManager* sprite_manager;
    TerrainRenderer* terrain_renderer_;
    std::unique_ptr<AudioManager> audio_manager;
    std::unique_ptr<Client> client;
    InputHandler input_handler;
    Camera camera;
    uint32_t my_player_id;
    uint8_t my_race;
    uint8_t my_klass;
    int player_x;
    int player_y;
    int width;
    int height;
    int my_hp;
    int my_max_hp;
    int my_mp;
    int my_max_mp;
    int my_level;
    uint64_t my_gold;
    uint64_t my_xp;
    bool my_is_ghost = false;
    std::map<uint32_t, PlayerSnapshot> players;
    std::vector<GroundItemSnapshot> ground_items_;
    std::vector<InventorySlotData> inventory_slots_;
    std::map<uint32_t, NPCSnapshot> npcs_;

    struct NPCAnimState {
        int current_frame = 0;
        Uint32 last_frame_time = 0;
        NPCVisualType sprite_type = NPCVisualType::UNKNOWN;
        uint8_t direction = 0;
        bool is_moving = false;
    };
    std::map<uint32_t, NPCAnimState> npc_anim_states_;

    bool chat_active_ = false;
    std::string chat_input_;
    int selected_slot_ = -1;
    int selected_npc_id_ = 0;

    // Run-loop state shared across extracted methods
    bool running_ = false;
    SDL_Event event_{};
    Uint32 frame_start_ = 0;
    bool moving_ = false;
    int direction_ = 0;
    int current_frame_ = 0;
    int total_frames_ = 6;
    Uint32 last_frame_time_ = 0;
    Uint32 last_move_time_ = 0;

 public:
    // Constructor standalone: crea y arranca su propio Client (binario argentum_client).
    GameClient(int width, int height, const std::string& host, const std::string& port);
    // Constructor de handoff Qt→SDL: recibe Client ya conectado y con lobby completado.
    GameClient(int width, int height, std::unique_ptr<Client> client, uint8_t race, uint8_t klass,
               uint32_t player_id);
    ~GameClient();

    GameClient(const GameClient&) = delete;
    GameClient& operator=(const GameClient&) = delete;

    void run();

 private:
    void load_audio_assets();
    void process_server_updates(int tile_w, int tile_h, ClientMap& client_map);
    void render_players(int tile_w, int tile_h, int direction, int current_frame);
    void render_npcs(int tile_w, int tile_h);
    void process_sdl_events();
    void process_keyword_input();
    void send_chat_message(const std::string& text);
    void toggle_chat();
    void process_chat_input(const SDL_Event& e);
};

#endif
