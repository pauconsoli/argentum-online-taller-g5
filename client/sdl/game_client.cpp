#include "game_client.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "client_map.h"
#include "common/updates/attack_update.h"
#include "common/updates/death_update.h"
#include "common/updates/error_update.h"
#include "common/updates/login_ok_update.h"
#include "common/updates/match_created_update.h"
#include "common/updates/match_joined_update.h"
#include "common/updates/moved_update.h"
#include "common/updates/snapshot_update.h"
#include "common/updates/world_map_update.h"
#include "server/game/player_class.h"
#include "server/game/player_race.h"

// Primera cabeza del rango de cada raza (HeadAndBodyData.json, male range start).
static uint16_t head_index_for_race(uint8_t race) {
    switch (race) {
        case 1:
            return 101;  // ELF
        case 2:
            return 300;  // DWARF
        case 3:
            return 400;  // GNOME
        default:
            return 1;  // HUMAN (y fallback)
    }
}

static void init_sdl_window(SDL_Window*& window, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        throw std::runtime_error(SDL_GetError());
    window = SDL_CreateWindow("Argentum Online - G5", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_Quit();
        throw std::runtime_error(SDL_GetError());
    }
}

GameClient::GameClient(int width, int height, const std::string& host, const std::string& port):
    window(nullptr),
    renderer(nullptr),
    hud(nullptr),
    mini_chat(nullptr),
    sprite_manager(nullptr),
    client(std::make_unique<Client>(host, port)),
    camera(width, height),
    my_player_id(0),
    my_race(0),
    my_klass(0),
    player_x(400),
    player_y(300),
    width(width),
    height(height),
    from_handoff(false) {
    init_sdl_window(window, width, height);
    my_hp = 100;
    my_max_hp = 100;
    my_mp = 100;
    my_max_mp = 100;
    my_level = 1;
    my_gold = 0;
    my_xp = 0;
    renderer = new Renderer(window);
    sprite_manager = new SpriteManager(renderer->get_sdl_renderer());
    sprite_manager->load_body_textures("client/assets");
    sprite_manager->load_terrain_textures("client/assets");
    hud = new Hud(renderer->get_sdl_renderer(), "client/assets/font.ttf", height, width);
    mini_chat = new MiniChat(renderer->get_sdl_renderer(), "client/assets/font.ttf", width);
    client->start();
}

GameClient::GameClient(int width, int height, std::unique_ptr<Client> c, uint8_t race,
                       uint8_t klass, uint32_t player_id):
    window(nullptr),
    renderer(nullptr),
    hud(nullptr),
    mini_chat(nullptr),
    sprite_manager(nullptr),
    client(std::move(c)),
    camera(width, height),
    my_player_id(player_id),
    my_race(race),
    my_klass(klass),
    player_x(400),
    player_y(300),
    width(width),
    height(height),
    from_handoff(true) {
    init_sdl_window(window, width, height);
    my_hp = 100;
    my_max_hp = 100;
    my_mp = 100;
    my_max_mp = 100;
    my_level = 1;
    my_gold = 0;
    my_xp = 0;
    renderer = new Renderer(window);
    sprite_manager = new SpriteManager(renderer->get_sdl_renderer());
    sprite_manager->load_body_textures("client/assets");
    sprite_manager->load_terrain_textures("client/assets");
    hud = new Hud(renderer->get_sdl_renderer(), "client/assets/font.ttf", height, width);
    mini_chat = new MiniChat(renderer->get_sdl_renderer(), "client/assets/font.ttf", width);
    // client->start() ya fue llamado por QtClientAdapter::start()
}

GameClient::~GameClient() {
    client->stop();
    client->join();
    delete hud;
    delete mini_chat;
    delete sprite_manager;
    delete renderer;
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void GameClient::run() {
    bool running = true;
    SDL_Event event;

    const int frame_w = 27;
    const int frame_h = 48;
    const int tile_w = 32;
    const int tile_h = 32;
    // cabezas: 27x256, 4 tiras de dirección de 64px (sin animación de frames)
    const int head_w = 27;
    const int head_h = 64;

    if (!from_handoff) {
        // Flujo standalone (taller_client): hace login y lobby localmente.
        client->do_login("player1");
        auto& q = client->get_received_updates();
        bool logged_in = false;
        bool in_match = false;
        uint32_t match_id = 0;

        while (running && !in_match) {
            while (SDL_PollEvent(&event)) {
                running = input_handler.handle(event);
            }
            std::unique_ptr<GameUpdate> u;
            while (q.try_pop(u)) {
                switch (u->get_type()) {
                    case UpdateType::LOGIN_OK:
                        my_player_id = static_cast<LoginOkUpdate&>(*u).player_id;
                        logged_in = true;
                        client->do_create_match("sala1", 4);
                        break;
                    case UpdateType::MATCH_CREATED:
                        match_id = static_cast<MatchCreatedUpdate&>(*u).match_id;
                        client->do_join_match(match_id);
                        break;
                    case UpdateType::MATCH_JOINED:
                        in_match = true;
                        client->do_select_race_class(static_cast<uint8_t>(PlayerRace::HUMAN),
                                                     static_cast<uint8_t>(PlayerClass::WARRIOR));
                        break;
                    default:
                        break;
                }
                if (in_match)
                    break;
            }
            (void) logged_in;
            SDL_Delay(10);
        }

        if (!running)
            return;
    }

    // TODO(chiaradelaurentis): ESTO LO TIENE QUE DECIDIR EL SERVIDOR!!!
    player_x = static_cast<int>(1 + my_player_id) * tile_w;
    player_y = tile_h;

    ClientMap client_map = build_sample_client_map();

    int direction = 0;
    int current_frame = 0;
    int total_frames = 6;
    Uint32 last_frame_time = SDL_GetTicks();
    const Uint32 frame_delay = 100;

    const Uint32 frame_time_ms = 1000 / 60;
    const Uint32 move_interval_ms = 200;  // máx 5 tiles/seg
    Uint32 last_move_time = 0;
    while (running) {
        Uint32 frame_start = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            if (!input_handler.handle(event))
                running = false;
            if (chat_active_) {
                if (event.type == SDL_TEXTINPUT) {
                    chat_input_ += event.text.text;
                } else if (event.type == SDL_KEYDOWN) {
                    switch (event.key.keysym.sym) {
                        case SDLK_RETURN:
                        case SDLK_RETURN2:
                            if (!chat_input_.empty()) {
                                if (chat_input_.rfind("/tomar", 0) == 0) {
                                    client->do_pick_up();
                                    mini_chat->add_message("/tomar");
                                } else {
                                    mini_chat->add_message(chat_input_);
                                }
                            }
                            chat_input_.clear();
                            chat_active_ = false;
                            SDL_StopTextInput();
                            break;
                        case SDLK_BACKSPACE:
                            if (!chat_input_.empty())
                                chat_input_.pop_back();
                            break;
                        case SDLK_ESCAPE:
                            chat_input_.clear();
                            chat_active_ = false;
                            SDL_StopTextInput();
                            break;
                        default:
                            break;
                    }
                }
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) {
                chat_active_ = true;
                SDL_StartTextInput();
            } else if (event.type == SDL_MOUSEBUTTONDOWN &&
                       event.button.button == SDL_BUTTON_LEFT) {
                if (!my_is_ghost) {
                    int world_x = camera.get_x() + event.button.x;
                    int world_y = camera.get_y() + event.button.y;
                    int tile_x = world_x / tile_w;
                    int tile_y = world_y / tile_h;
                    for (const auto& [pid, ps] : players) {
                        if (pid != my_player_id && ps.x == tile_x && ps.y == tile_y) {
                            client->do_attack(pid);
                            break;
                        }
                    }
                    int slot = hud->get_slot_at(event.button.x, event.button.y);
                    if (slot >= 0 && slot < static_cast<int>(inventory_slots_.size()) &&
                        !inventory_slots_[slot].item_name.empty()) {
                        client->do_equip_item(static_cast<uint8_t>(slot));
                    }
                }
            }
        }

        const Uint8* keys = SDL_GetKeyboardState(nullptr);
        bool moving = false;
        if (!chat_active_) {
            bool can_move = (frame_start - last_move_time >= move_interval_ms);
            if (keys[SDL_SCANCODE_DOWN]) {
                direction = 0;
                total_frames = 6;
                moving = true;
                if (can_move) {
                    client->do_move(Direction::DOWN);
                    last_move_time = frame_start;
                }
            } else if (keys[SDL_SCANCODE_UP]) {
                direction = 1;
                total_frames = 6;
                moving = true;
                if (can_move) {
                    client->do_move(Direction::UP);
                    last_move_time = frame_start;
                }
            } else if (keys[SDL_SCANCODE_LEFT]) {
                direction = 2;
                total_frames = 5;
                moving = true;
                if (can_move) {
                    client->do_move(Direction::LEFT);
                    last_move_time = frame_start;
                }
            } else if (keys[SDL_SCANCODE_RIGHT]) {
                direction = 3;
                total_frames = 5;
                moving = true;
                if (can_move) {
                    client->do_move(Direction::RIGHT);
                    last_move_time = frame_start;
                }
            }
        }

        auto& update_queue = client->get_received_updates();
        std::unique_ptr<GameUpdate> update;
        while (update_queue.try_pop(update)) {
            switch (update->get_type()) {
                case UpdateType::ERROR: {
                    const auto& eu = static_cast<const ErrorUpdate&>(*update);
                    const auto& d = eu.detail;
                    if (d.find("move_player") != std::string::npos ||
                        d.find("mover") != std::string::npos) {
                        break;
                    }
                    if (d.find("muertos") != std::string::npos ||
                        d.find("muerto") != std::string::npos ||
                        d.find("ghost") != std::string::npos ||
                        d.find("fantasma") != std::string::npos) {
                        mini_chat->add_message("No puedes atacar a un jugador muerto");
                    } else if (d.find("objetivo") != std::string::npos ||
                               d.find("target") != std::string::npos ||
                               d.find("attack") != std::string::npos) {
                        mini_chat->add_message("Debes estar más cerca para atacar");
                    } else {
                        mini_chat->add_message("Error: " + d);
                    }
                    break;
                }
                case UpdateType::SNAPSHOT: {
                    const auto& snap = static_cast<const SnapshotUpdate&>(*update);
                    players.clear();
                    for (const auto& ps : snap.players) {
                        players[ps.player_id] = ps;
                        if (ps.player_id == my_player_id) {
                            player_x = ps.x * tile_w;  // tiles → píxeles
                            player_y = ps.y * tile_h;
                            my_race = ps.race;
                            my_klass = ps.klass;
                            my_hp = ps.hp;
                            my_mp = ps.mp;
                            my_max_hp = ps.max_hp;
                            my_max_mp = ps.max_mp;
                            my_level = ps.level;
                            my_gold = ps.gold;
                            my_xp = ps.xp;
                            my_is_ghost = ps.is_ghost;
                        }
                    }
                    ground_items_ = snap.ground_items;
                    break;
                }
                case UpdateType::WORLD_MAP: {
                    const auto& mu = static_cast<const WorldMapUpdate&>(*update);
                    std::vector<MapCell> map_cells;
                    map_cells.reserve(mu.cells.size());
                    std::transform(
                        mu.cells.begin(), mu.cells.end(), std::back_inserter(map_cells),
                        [](const auto& c) {
                            return MapCell{static_cast<TerrainType>(c.terrain_type), c.blocking};
                        });
                    client_map = ClientMap(mu.width, mu.height, std::move(map_cells));
                    break;
                }
                case UpdateType::INVENTORY: {
                    const auto& iu = static_cast<const InventoryUpdate&>(*update);
                    inventory_slots_ = iu.get_items();
                    my_gold = iu.get_gold();
                    std::cout << "[INV] " << inventory_slots_.size() << " slots:\n";
                    for (size_t i = 0; i < inventory_slots_.size(); i++) {
                        const auto& s = inventory_slots_[i];
                        if (!s.item_name.empty())
                            std::cout << "  [" << i << "] " << s.item_name << " x" << s.quantity
                                      << (s.is_equipped ? " (equipado)" : "") << "\n";
                    }
                    std::cout << std::flush;
                    break;
                }
                case UpdateType::ATTACKED: {
                    const auto& au = static_cast<const AttackUpdate&>(*update);
                    const AttackResult& r = au.get_result();
                    if (r.evaded) {
                        mini_chat->add_message("Ataque esquivado");
                    } else if (r.attacker_id == my_player_id) {
                        if (r.target_died)
                            mini_chat->add_message("Mataste al jugador");
                        else
                            mini_chat->add_message("Causaste " + std::to_string(r.damage) +
                                                   " de daño");
                    } else if (r.target_id == my_player_id) {
                        if (r.target_died)
                            mini_chat->add_message("Moriste");
                        else
                            mini_chat->add_message("Recibiste " + std::to_string(r.damage) +
                                                   " de daño");
                    }
                    break;
                }
                case UpdateType::DEATH: {
                    const auto& du = static_cast<const DeathUpdate&>(*update);
                    if (du.get_dead_id() == my_player_id)
                        mini_chat->add_message("Moriste. Dirigete al sanador para resucitar.");
                    else
                        mini_chat->add_message("Un jugador murio en combate.");
                    break;
                }
                default:
                    break;
            }
        }

        if (moving) {
            Uint32 now = SDL_GetTicks();
            if (now - last_frame_time > frame_delay) {
                current_frame = (current_frame + 1) % total_frames;
                last_frame_time = now;
            }
        } else {
            current_frame = 0;
        }

        // centra la camara en el jugador local
        camera.center_on(player_x, player_y, client_map.get_width() * tile_w,
                         client_map.get_height() * tile_h);

        renderer->clear();

        int start_col = camera.get_x() / tile_w;
        int end_col = start_col + width / tile_w + 1;
        int start_row = camera.get_y() / tile_h;
        int end_row = start_row + height / tile_h + 1;
        for (int row = start_row; row <= end_row; row++) {
            for (int col = start_col; col <= end_col; col++) {
                int tx = camera.get_screen_x(col * tile_w);
                int ty = camera.get_screen_y(row * tile_h);
                bool in_bounds = (col >= 0 && col < client_map.get_width() && row >= 0 &&
                                  row < client_map.get_height());
                TerrainType terrain =
                    in_bounds ? client_map.at(col, row).terrain : TerrainType::GRASS;
                bool blocking = in_bounds && client_map.at(col, row).blocking;
                SDL_Texture* tile_tex = sprite_manager->get_terrain(terrain);
                renderer->draw_frame(tile_tex, 0, 0, tile_w, tile_h, tx, ty);
                // Árbol (GRASS + blocking): sprite extraído de Recursos/Graficos/657.png (grh=653)
                if (blocking && terrain == TerrainType::GRASS) {
                    renderer->draw_frame(sprite_manager->get_tree(), 0, 0, tile_w, tile_h, tx, ty);
                }
            }
        }

        // Ground items: entre terreno y jugadores
        for (const auto& gi : ground_items_) {
            std::string item_key =
                gi.is_gold ? "item_2" : SpriteManager::item_key_for_name(gi.name);
            SDL_Texture* item_tex = sprite_manager->get_item(item_key);
            if (item_tex == nullptr)
                item_tex = sprite_manager->get_item("item_2");
            if (item_tex != nullptr) {
                int gx = camera.get_screen_x(gi.x * tile_w);
                int gy = camera.get_screen_y(gi.y * tile_h);
                renderer->draw_frame(item_tex, 0, 0, tile_w, tile_h, gx, gy);
            }
        }

        // Dibuja todos los jugadores con el body según raza/clase del snapshot
        static const int head_offset_x[] = {0, 0, 0, 0};
        static const int head_offset_y[] = {-8, -15, -15, -15};
        // El spritesheet de cabezas tiene filas en orden: UP(0),RIGHT(1),DOWN(2),LEFT(3)
        // pero direction usa: 0=DOWN,1=UP,2=LEFT,3=RIGHT → necesita remapeo
        static const int head_dir_to_row[] = {2, 0, 3, 1};

        for (const auto& [pid, ps] : players) {
            int px = camera.get_screen_x(ps.x * tile_w);  // tiles → píxeles
            int py = camera.get_screen_y(ps.y * tile_h);

            int p_dir = (pid == my_player_id) ? direction : 0;
            int p_frame = (pid == my_player_id) ? current_frame : 0;
            int p_total = (p_dir < 2) ? 6 : 5;
            int p_frame_clamped = p_frame % p_total;

            int p_frame_x = p_frame_clamped * frame_w;
            int p_frame_y = p_dir * frame_h;

            SDL_Texture* body_tex = sprite_manager->get_body(ps.race, ps.klass);
            SDL_Texture* head_tex = sprite_manager->get_head(head_index_for_race(ps.race));
            uint8_t alpha = ps.is_ghost ? 128 : 255;
            SDL_SetTextureAlphaMod(body_tex, alpha);
            SDL_SetTextureAlphaMod(head_tex, alpha);

            renderer->draw_frame(body_tex, p_frame_x, p_frame_y, frame_w, frame_h, px, py);

            int hx = px + head_offset_x[p_dir];
            int hy = py + head_offset_y[p_dir];
            renderer->draw_frame(head_tex, 0, head_dir_to_row[p_dir] * head_h, head_w, head_h, hx,
                                 hy);

            SDL_SetTextureAlphaMod(body_tex, 255);
            SDL_SetTextureAlphaMod(head_tex, 255);

            // Nick centrado debajo de los pies del sprite (sombra + texto blanco)
            int nick_center_x = px + frame_w / 2;
            int nick_y = py + frame_h + 3;
            SDL_Color shadow = {0, 0, 0, 200};
            SDL_Color white = {255, 255, 255, 255};
            mini_chat->draw_label(ps.nick, nick_center_x + 1, nick_y + 1, shadow);
            mini_chat->draw_label(ps.nick, nick_center_x, nick_y, white);
        }

        hud->draw(my_hp, my_max_hp, my_mp, my_max_mp, my_level, my_gold, my_xp);
        hud->draw_inventory(sprite_manager, inventory_slots_);

        mini_chat->draw();
        if (chat_active_)
            mini_chat->draw_input(chat_input_, height - 30);
        renderer->present();

        Uint32 elapsed = SDL_GetTicks() - frame_start;
        if (elapsed < frame_time_ms) {
            SDL_Delay(frame_time_ms - elapsed);
        }
    }
}
