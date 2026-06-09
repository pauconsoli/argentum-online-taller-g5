#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <SDL2/SDL.h>

#include "../client.h"
#include "camera.h"
#include "common/updates/inventory_update.h"
#include "common/updates/snapshot_update.h"
#include "hud.h"
#include "input_handler.h"
#include "mini_chat.h"
#include "renderer.h"
#include "sprite_manager.h"

class GameClient {
 private:
    SDL_Window* window;
    Renderer* renderer;
    Hud* hud;
    MiniChat* mini_chat;
    SpriteManager* sprite_manager;
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
    bool from_handoff;
    bool my_is_ghost = false;
    std::map<uint32_t, PlayerSnapshot> players;
    std::vector<GroundItemSnapshot> ground_items_;
    std::vector<InventorySlotData> inventory_slots_;
    bool chat_active_ = false;
    std::string chat_input_;
    int selected_slot_ = -1;

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
};

#endif
