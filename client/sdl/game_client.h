#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

class ClientMap;
class WorldRenderer;

#include <cstdint>
#include <memory>
#include <string>

#include <SDL2/SDL.h>

#include "../client.h"
#include "audio_manager.h"
#include "camera.h"
#include "character_renderer.h"
#include "clan_panel.h"
#include "help_menu.h"
#include "hud.h"
#include "input_handler.h"
#include "mini_chat.h"
#include "npc_renderer.h"
#include "player_renderer.h"
#include "renderer.h"
#include "sdl_config.h"
#include "sdl_game_state.h"
#include "server_update_handler.h"
#include "sprite_manager.h"
#include "terrain_renderer.h"

class GameClient: public Notifier {
 private:
    // Deleter que cierra ventana y termina SDL juntos.
    // Declarado antes que renderer para que renderer se destruya primero
    // (orden de destrucción = inverso al de declaración).
    struct SdlWindowDeleter {
        void operator()(SDL_Window* w) const noexcept {
            SDL_DestroyWindow(w);
            SDL_Quit();
        }
    };

    SdlConfig config;
    std::unique_ptr<SDL_Window, SdlWindowDeleter> window;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<CharacterRenderer> character_renderer;
    std::unique_ptr<HelpMenu> help_menu;
    std::unique_ptr<ClanPanel> clan_panel;
    std::unique_ptr<Hud> hud;
    std::unique_ptr<MiniChat> mini_chat;
    std::unique_ptr<SpriteManager> sprite_manager;
    std::unique_ptr<TerrainRenderer> terrain_renderer;
    std::unique_ptr<NPCRenderer> npc_renderer;
    std::unique_ptr<PlayerRenderer> player_renderer;
    std::unique_ptr<AudioManager> audio_manager;
    std::unique_ptr<Client> client;
    InputHandler input_handler;
    Camera camera;
    int width;
    int height;

    GameState state;
    ServerUpdateHandler update_handler;
    std::unique_ptr<WorldRenderer> world_renderer;

    bool chat_active = false;
    bool music_paused = false;
    std::string chat_input;

    // Run-loop state shared across extracted methods
    bool running = false;
    SDL_Event event{};
    Uint32 frame_start = 0;
    bool moving = false;
    int direction = 0;
    int current_frame = 0;
    int total_frames = 6;
    Uint32 last_frame_time = 0;
    Uint32 last_move_time = 0;

 public:
    // Constructor standalone: crea y arranca su propio Client (binario argentum_client).
    GameClient(int width, int height, bool fullscreen, const std::string& host,
               const std::string& port);
    // Constructor de handoff Qt→SDL: recibe Client ya conectado y con lobby completado.
    GameClient(int width, int height, bool fullscreen, std::unique_ptr<Client> client, uint8_t race,
               uint8_t klass, uint32_t player_id);
    ~GameClient() override;

    GameClient(const GameClient&) = delete;
    GameClient& operator=(const GameClient&) = delete;

    void run();
    void message(const std::string& text) override;
    void play(const std::string& sound, int vol = -1) override;
    void show_clan_review(const ClanReviewUpdate& cru) override;

 private:
    void init_subsystems(bool fullscreen, bool load_font);
    void load_audio_assets();
    void process_sdl_events();
    void handle_chat_event(const SDL_Event& e);
    void handle_chat_submit();
    void dispatch_chat_command(const std::string& input);
    void handle_drop_command(const std::string& input);
    void handle_npc_command(NPCInteraction type, const std::string& arg, int32_t amount);
    void handle_left_click(int screen_x, int screen_y);
    bool try_attack_at_tile(int tile_x, int tile_y);
    void try_equip_at(int screen_x, int screen_y);
    void handle_right_click(int screen_x, int screen_y);
    void process_keyword_input();
    void send_chat_message(const std::string& text);
    void toggle_chat();
    void process_chat_input(const SDL_Event& e);
};

#endif
