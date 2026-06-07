#ifndef GAME_CLIENT_H
#define GAME_CLIENT_H

#include <cstdint>
#include <map>
#include <string>

#include <SDL2/SDL.h>

#include "../client.h"
#include "camera.h"
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
    Client client;
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
    std::map<uint32_t, PlayerSnapshot> players;

 public:
    GameClient(int width, int height, const std::string& host, const std::string& port);
    ~GameClient();

    GameClient(const GameClient&) = delete;
    GameClient& operator=(const GameClient&) = delete;

    void run();
};

#endif
