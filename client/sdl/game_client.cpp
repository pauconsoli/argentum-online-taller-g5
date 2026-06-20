#include "game_client.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "client_map.h"
#include "common/updates/attack_update.h"
#include "common/updates/chat_msg_update.h"
#include "common/updates/death_update.h"
#include "common/updates/error_update.h"
#include "common/updates/login_ok_update.h"
#include "common/updates/match_created_update.h"
#include "common/updates/match_joined_update.h"
#include "common/updates/moved_update.h"
#include "common/updates/player_left_update.h"
#include "common/updates/snapshot_update.h"
#include "common/updates/world_map_update.h"
#include "server/game/player_class.h"
#include "server/game/player_race.h"

// esto deberia estar en sprite manager??
static std::string get_base_asset_dir() {
    if (const char* env_dir = std::getenv("ARGENTUM_DATA_DIR")) {
        return std::string(env_dir);
    }
    return "assets";
}

// inicializa video + audio
// esto deberia ir en audio manager o esta ok aca?
static void init_sdl_window(SDL_Window*& window, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
        throw std::runtime_error(SDL_GetError());
    window = SDL_CreateWindow("Argentum Online - G5", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_Quit();
        throw std::runtime_error(SDL_GetError());
    }
}

// Constructor standalone
GameClient::GameClient(int width, int height, const std::string& host, const std::string& port):
    window(nullptr),
    renderer(nullptr),
    character_renderer(nullptr),
    hud(nullptr),
    mini_chat(nullptr),
    sprite_manager(nullptr),
    terrain_renderer_(nullptr),
    npc_renderer_(nullptr),
    audio_manager(nullptr),
    client(std::make_unique<Client>(host, port)),
    camera(width, height),
    my_player_id(1),  // ID provisorio/dummy para standalone
    my_race(1),
    my_klass(1),
    player_x(400),
    player_y(300),
    width(width),
    height(height) {
    init_sdl_window(window, width, height);
    my_hp = 100;
    my_max_hp = 100;
    my_mp = 100;
    my_max_mp = 100;
    my_level = 1;
    my_gold = 0;
    my_xp = 0;
    renderer = new Renderer(window);
    character_renderer = new CharacterRenderer(renderer);
    std::string base_assets = get_base_asset_dir();
    std::string font_path = base_assets + "/fonts/font.ttf";
    sprite_manager = new SpriteManager(renderer->get_sdl_renderer());
    sprite_manager->load_body_textures(base_assets);
    sprite_manager->load_terrain_textures(base_assets);
    terrain_renderer_ = new TerrainRenderer(renderer, sprite_manager, camera);
    npc_renderer_ = new NPCRenderer(renderer, sprite_manager, camera);
    player_renderer_ = new PlayerRenderer(character_renderer, sprite_manager, camera);
    hud = new Hud(renderer->get_sdl_renderer(), font_path, height, width);
    mini_chat = new MiniChat(renderer->get_sdl_renderer(), font_path, width);
    audio_manager = std::make_unique<AudioManager>();
    load_audio_assets();
}


GameClient::GameClient(int width, int height, std::unique_ptr<Client> c, uint8_t race,
                       uint8_t klass, uint32_t player_id):
    config_(SdlConfig::load("client/config/sdl_config.toml")),
    window(nullptr),
    renderer(nullptr),
    hud(nullptr),
    mini_chat(nullptr),
    sprite_manager(nullptr),
    terrain_renderer_(nullptr),
    npc_renderer_(nullptr),
    audio_manager(nullptr),
    client(std::move(c)),
    camera(width, height),
    my_player_id(player_id),
    my_race(race),
    my_klass(klass),
    player_x(400),
    player_y(300),
    width(width),
    height(height) {
    init_sdl_window(window, width, height);
    my_hp = 100;
    my_max_hp = 100;
    my_mp = 100;
    my_max_mp = 100;
    my_level = 1;
    my_gold = 0;
    my_xp = 0;
    renderer = new Renderer(window);
    character_renderer = new CharacterRenderer(renderer);
    std::string base_assets = get_base_asset_dir();
    std::string font_path = base_assets + "/fonts/font.ttf";
    renderer->load_font(font_path, 12);
    sprite_manager = new SpriteManager(renderer->get_sdl_renderer());
    sprite_manager->load_body_textures(base_assets);
    sprite_manager->load_terrain_textures(base_assets);
    terrain_renderer_ = new TerrainRenderer(renderer, sprite_manager, camera);
    npc_renderer_ = new NPCRenderer(renderer, sprite_manager, camera);
    player_renderer_ = new PlayerRenderer(character_renderer, sprite_manager, camera);
    hud = new Hud(renderer->get_sdl_renderer(), font_path, height, width);
    mini_chat = new MiniChat(renderer->get_sdl_renderer(), font_path, width);
    audio_manager = std::make_unique<AudioManager>();
    load_audio_assets();
}

GameClient::~GameClient() {
    client->stop();
    client->join();
    delete character_renderer;
    delete hud;
    delete mini_chat;
    delete terrain_renderer_;
    delete npc_renderer_;
    delete player_renderer_;
    delete sprite_manager;
    audio_manager.reset();
    delete renderer;
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void GameClient::run() {
    running_ = true;

    audio_manager->play_background_music(get_base_asset_dir() + "/audio/music/background.mp3",
                                         MIX_MAX_VOLUME / 2);

    ClientMap client_map = build_sample_client_map();

    // esto que es? porque se declara asi?
    player_x = -1;
    player_y = -1;

    direction_ = 0;
    current_frame_ = 0;
    total_frames_ = 6;
    last_frame_time_ = SDL_GetTicks();
    last_move_time_ = 0;

    while (running_) {
        frame_start_ = SDL_GetTicks();

        process_sdl_events();
        process_keyword_input();
        process_server_updates(config_.tile_width, config_.tile_height, client_map);


        if (moving_) {
            Uint32 now = SDL_GetTicks();
            if (now - last_frame_time_ > config_.frame_delay_ms) {
                current_frame_ = (current_frame_ + 1) % total_frames_;
                last_frame_time_ = now;
            }
        } else {
            current_frame_ = 0;
        }

        if (player_x >= 0 && player_y >= 0) {
            camera.center_on(player_x, player_y, client_map.get_width() * config_.tile_width,
                             client_map.get_height() * config_.tile_height);
        }

        renderer->clear();

        int start_col = camera.get_x() / config_.tile_width;
        int end_col = start_col + width / config_.tile_width + 1;
        int start_row = camera.get_y() / config_.tile_height;
        int end_row = start_row + height / config_.tile_height + 1;
        terrain_renderer_->draw(start_col, end_col, start_row, end_row, config_.tile_width,
                                config_.tile_height, client_map);

        for (const auto& gi : ground_items_) {
            SDL_Texture* item_tex = nullptr;
            if (gi.is_gold) {
                item_tex = sprite_manager->get_gold();
            } else {
                std::string item_key = SpriteManager::item_key_for_name(gi.name);
                item_tex = sprite_manager->get_item(item_key);
            }
            if (item_tex == nullptr)
                item_tex = sprite_manager->get_item("item_espada");
            if (item_tex != nullptr) {
                int gx = camera.get_screen_x(gi.x * config_.tile_width);
                int gy = camera.get_screen_y(gi.y * config_.tile_height);
                renderer->draw_frame(item_tex, 0, 0, config_.tile_width, config_.tile_height, gx,
                                     gy);
            }
        }

        player_renderer_->render(players, my_player_id, direction_, current_frame_,
                                 inventory_slots_, config_.tile_width, config_.tile_height);
        npc_renderer_->render(npcs_, config_.tile_width, config_.tile_height);

        hud->draw(my_hp, my_max_hp, my_mp, my_max_mp, my_level, my_gold, my_xp);
        hud->draw_inventory(sprite_manager, inventory_slots_, selected_slot_);

        mini_chat->draw();
        if (chat_active_)
            mini_chat->draw_input(chat_input_, height - 30);
        renderer->present();

        Uint32 elapsed = SDL_GetTicks() - frame_start_;
        if (elapsed < config_.frame_time_ms()) {
            SDL_Delay(config_.frame_time_ms() - elapsed);
        }
    }
}

// Vacía la cola de updates del servidor y actualiza el estado local según el tipo de mensaje.
void GameClient::process_server_updates(int tile_w, int tile_h, ClientMap& client_map) {
    auto& update_queue = client->get_received_updates();
    std::unique_ptr<GameUpdate> update;
    while (update_queue.try_pop(update)) {
        switch (update->get_type()) {
            case UpdateType::ERROR:
                handle_error_update(static_cast<const ErrorUpdate&>(*update));
                break;
            case UpdateType::SNAPSHOT:
                handle_snapshot_update(static_cast<const SnapshotUpdate&>(*update), tile_w, tile_h,
                                       client_map);
                break;
            case UpdateType::PLAYER_LEFT:
                handle_player_left_update(static_cast<const PlayerLeftUpdate&>(*update));
                break;
            case UpdateType::WORLD_MAP:
                handle_world_map_update(static_cast<const WorldMapUpdate&>(*update), client_map);
                break;
            case UpdateType::INVENTORY:
                handle_inventory_update(static_cast<const InventoryUpdate&>(*update));
                break;
            case UpdateType::ATTACKED:
                handle_attack_update(static_cast<const AttackUpdate&>(*update));
                break;
            case UpdateType::DEATH:
                handle_death_update(static_cast<const DeathUpdate&>(*update), tile_w, tile_h);
                break;
            case UpdateType::CHAT_MSG:
                handle_chat_msg_update(static_cast<const ChatMsgUpdate&>(*update));
                break;
            default:
                break;
        }
    }
}

void GameClient::handle_error_update(const ErrorUpdate& eu) {
    const auto& d = eu.detail;
    if (d.find("move_player") != std::string::npos || d.find("mover") != std::string::npos)
        return;
    if (d.find("muertos") != std::string::npos || d.find("muerto") != std::string::npos ||
        d.find("ghost") != std::string::npos || d.find("fantasma") != std::string::npos) {
        mini_chat->add_message("No puedes atacar a un jugador muerto");
    } else if (d.find("objetivo") != std::string::npos || d.find("target") != std::string::npos ||
               d.find("attack") != std::string::npos) {
        mini_chat->add_message("Debes estar más cerca para atacar");
    } else {
        mini_chat->add_message("Error: " + d);
    }
}

void GameClient::handle_snapshot_update(const SnapshotUpdate& snap, int tile_w, int tile_h,
                                        ClientMap& /*client_map*/) {
    players.clear();
    for (const auto& ps : snap.players) {
        players[ps.player_id] = ps;
        if (ps.player_id == my_player_id) {
            player_x = ps.x * tile_w;
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
    npcs_.clear();
    for (const auto& ns : snap.npcs) npcs_[ns.npc_id] = ns;
    npc_renderer_->sync_from_snapshot(snap.npcs);
}

void GameClient::handle_player_left_update(const PlayerLeftUpdate& pu) {
    players.erase(pu.player_id);
}

void GameClient::handle_world_map_update(const WorldMapUpdate& mu, ClientMap& client_map) {
    std::vector<MapCell> map_cells;
    map_cells.reserve(mu.cells.size());
    std::transform(mu.cells.begin(), mu.cells.end(), std::back_inserter(map_cells),
                   [](const auto& c) {
                       return MapCell{static_cast<TerrainType>(c.terrain_type), c.blocking};
                   });
    client_map = ClientMap(mu.width, mu.height, std::move(map_cells));
}

void GameClient::handle_inventory_update(const InventoryUpdate& inventory_updated) {
    inventory_slots_ = inventory_updated.get_items();
    my_gold = inventory_updated.get_gold();
}

void GameClient::handle_attack_update(const AttackUpdate& atack_updated) {
    const AttackResult& result = atack_updated.get_result();
    if (result.evaded) {
        mini_chat->add_message("Ataque esquivado");
    } else if (result.attacker_id == my_player_id) {
        std::string target_name;
        auto player_it = players.find(result.target_id);
        if (player_it != players.end()) {
            target_name = player_it->second.nick;
        } else {
            auto npc_it = npcs_.find(result.target_id);
            if (npc_it != npcs_.end())
                target_name = npc_it->second.name;
        }
        if (result.is_healing) {
            mini_chat->add_message(result.weapon_or_spell_name + ": curaste a " + target_name +
                                   " por " + std::to_string(result.heal_amount) + " puntos");
        } else if (!result.target_died) {
            mini_chat->add_message(result.weapon_or_spell_name + ": causaste " +
                                   std::to_string(result.damage) + " de daño a " + target_name);
        }
    } else if (result.target_id == my_player_id) {
        if (result.is_healing) {
            std::string healer_name;
            auto player_it = players.find(result.attacker_id);
            if (player_it != players.end()) {
                healer_name = player_it->second.nick;
            } else {
                auto npc_it = npcs_.find(result.attacker_id);
                if (npc_it != npcs_.end())
                    healer_name = npc_it->second.name;
            }
            mini_chat->add_message(healer_name + " te curó " + std::to_string(result.heal_amount) +
                                   " puntos de vida");
        } else if (!result.target_died) {
            mini_chat->add_message("Recibiste " + std::to_string(result.damage) + " de daño");
        }
    }
    play_attack_sound(result);
}

void GameClient::play_attack_sound(const AttackResult& result) {
    switch (result.type) {
        case AttackType::NORMAL:
            if (!result.evaded && result.damage > 0)
                audio_manager->play_sound("melee_hit");
            break;
        case AttackType::RANGED:
            audio_manager->play_sound("ranged_attack");
            break;
        case AttackType::MAGIC:
            if (!result.is_healing) {
                std::string lname = result.weapon_or_spell_name;
                std::transform(lname.begin(), lname.end(), lname.begin(),
                               [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                if (lname.find("explos") != std::string::npos)
                    audio_manager->play_sound("explosion");
                // magic_attack.wav no existe; otros hechizos sin audio por ahora
            }
            // heal.wav no existe en assets; curacion sin audio por ahora
            break;
    }
}

void GameClient::handle_death_update(const DeathUpdate& death_updated, int tile_w, int tile_h) {
    if (death_updated.get_dead_id() == my_player_id) {
        mini_chat->add_message("Moriste. Dirigite al sacerdote para resucitar");
        audio_manager->play_sound("death");
        return;
    }

    int vol = MIX_MAX_VOLUME;
    uint32_t dead_id = death_updated.get_dead_id();
    int dead_tx = -1, dead_ty = -1;

    auto pit = players.find(dead_id);
    if (pit != players.end()) {
        const auto& snap = pit->second;
        dead_tx = snap.x;
        dead_ty = snap.y;
        if (death_updated.get_killer_id() == my_player_id)
            mini_chat->add_message("Mataste a " + snap.nick);
        else
            mini_chat->add_message("Un jugador murio en combate");
    } else {
        auto nit = npcs_.find(dead_id);
        if (nit != npcs_.end()) {
            const auto& snap = nit->second;
            dead_tx = snap.x;
            dead_ty = snap.y;
            if (death_updated.get_killer_id() == my_player_id)
                mini_chat->add_message("Mataste a " + snap.name);
            else
                mini_chat->add_message("Un NPC murió en combate");
        }
    }

    if (dead_tx >= 0) {
        int dx = dead_tx - player_x / tile_w;
        int dy = dead_ty - player_y / tile_h;
        float dist = std::sqrt(static_cast<float>(dx * dx + dy * dy));
        constexpr float kMaxDist = 20.0f;
        vol = (dist >= kMaxDist) ? 0 : static_cast<int>((1.0f - dist / kMaxDist) * MIX_MAX_VOLUME);
    }
    audio_manager->play_sound("death", vol);
}

void GameClient::handle_chat_msg_update(const ChatMsgUpdate& msg) {
    mini_chat->add_message(msg.sender_nick + ": " + msg.text);
}


// Carga todos los efectos de sonido del juego en el AudioManager.
void GameClient::load_audio_assets() {
    const std::string audio_path = get_base_asset_dir() + "/audio/sfx/";

    audio_manager->load_sound("melee_hit", audio_path + "melee_hit.wav");
    audio_manager->load_sound("ranged_attack", audio_path + "ranged_attack.wav");
    audio_manager->load_sound("explosion", audio_path + "explosion.wav");
    audio_manager->load_sound("death", audio_path + "death.wav");
    audio_manager->load_sound("drink_potion", audio_path + "drink_potion.wav");
}


void GameClient::process_sdl_events() {
    while (SDL_PollEvent(&event_)) {
        if (!input_handler.handle_quit(event_))
            running_ = false;

        if (chat_active_) {
            if (event_.type == SDL_TEXTINPUT) {
                chat_input_ += event_.text.text;
            } else if (event_.type == SDL_KEYDOWN) {
                switch (event_.key.keysym.sym) {
                    case SDLK_RETURN:
                    case SDLK_RETURN2:
                        if (!chat_input_.empty()) {
                            if (chat_input_.rfind("/tomar", 0) == 0) {
                                client->do_pick_up();
                                mini_chat->add_message("/tomar");
                            } else if (chat_input_.rfind("/tirar", 0) == 0) {
                                std::string rest = chat_input_.substr(6);
                                size_t pos = rest.find_first_not_of(' ');
                                int slot = -1;
                                if (pos != std::string::npos) {
                                    try {
                                        slot = std::stoi(rest.substr(pos));
                                    } catch (...) {
                                        slot = -1;
                                    }
                                } else {
                                    slot = selected_slot_;
                                }
                                if (slot >= 0 && slot < static_cast<int>(inventory_slots_.size()) &&
                                    !inventory_slots_[slot].item_name.empty()) {
                                    client->do_drop_item(static_cast<uint8_t>(slot));
                                    mini_chat->add_message("Tiraste el item");
                                    selected_slot_ = -1;
                                } else if (pos == std::string::npos && selected_slot_ < 0) {
                                    mini_chat->add_message(
                                        "Seleccioná un item con click derecho primero");
                                } else {
                                    mini_chat->add_message("Slot inválido o vacío");
                                }
                            } else if (chat_input_ == "/curar" || chat_input_ == "/resucitar") {
                                if (selected_npc_id_ == 0) {
                                    mini_chat->add_message("Seleccioná un sacerdote primero");
                                } else {
                                    NPCInteraction tipo = (chat_input_ == "/curar") ?
                                                              NPCInteraction::HEAL :
                                                              NPCInteraction::RESURRECT;
                                    client->do_interact(static_cast<uint32_t>(selected_npc_id_),
                                                        tipo, "", 0);
                                    mini_chat->add_message(chat_input_);
                                }
                            } else {
                                client->do_chat(chat_input_);
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
        } else if (event_.type == SDL_KEYDOWN && event_.key.keysym.sym == SDLK_RETURN) {
            chat_active_ = true;
            SDL_StartTextInput();
        } else if (event_.type == SDL_MOUSEBUTTONDOWN && event_.button.button == SDL_BUTTON_LEFT) {
            if (!my_is_ghost) {
                int world_x = camera.get_x() + event_.button.x;
                int world_y = camera.get_y() + event_.button.y;
                int tile_x = world_x / config_.tile_width;
                int tile_y = world_y / config_.tile_height;
                for (const auto& [pid, ps] : players) {
                    if (pid != my_player_id && ps.x == tile_x && ps.y == tile_y) {
                        client->do_attack(pid);
                        break;
                    }
                }
                for (const auto& [nid, ns] : npcs_) {
                    if (ns.x == tile_x && ns.y == tile_y) {
                        if (!ns.is_hostile) {
                            selected_npc_id_ = static_cast<int>(nid);
                            NPCVisualType npc_city = npc_visual_type_from_network(ns.npc_type);
                            switch (npc_city) {
                                case NPCVisualType::PRIEST:
                                    mini_chat->add_message("Seleccionaste al sacerdote");
                                    break;
                                case NPCVisualType::MERCHANT:
                                    mini_chat->add_message("Seleccionaste al mercader");
                                    break;
                                case NPCVisualType::BANKER:
                                    mini_chat->add_message("Seleccionaste al banquero");
                                    break;
                                default:
                                    mini_chat->add_message("Seleccionaste a un NPC");
                                    break;
                            }
                        } else {
                            client->do_attack(nid);
                        }
                        break;
                    }
                }
                int slot = hud->get_slot_at(event_.button.x, event_.button.y);
                if (slot >= 0 && slot < static_cast<int>(inventory_slots_.size()) &&
                    !inventory_slots_[slot].item_name.empty()) {
                    client->do_equip_item(static_cast<uint8_t>(slot));
                }
            }
        } else if (event_.type == SDL_MOUSEBUTTONDOWN && event_.button.button == SDL_BUTTON_RIGHT) {
            int slot = hud->get_slot_at(event_.button.x, event_.button.y);
            if (slot >= 0 && slot < static_cast<int>(inventory_slots_.size()) &&
                !inventory_slots_[slot].item_name.empty()) {
                selected_slot_ = slot;
            }
        }
    }
}

void GameClient::process_keyword_input() {
    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    moving_ = false;
    if (!chat_active_) {
        bool can_move = (frame_start_ - last_move_time_ >= config_.move_interval_ms);
        if (keys[SDL_SCANCODE_DOWN]) {
            direction_ = 0;
            total_frames_ = 6;
            moving_ = true;
            if (can_move) {
                client->do_move(Direction::DOWN);
                last_move_time_ = frame_start_;
            }
        } else if (keys[SDL_SCANCODE_UP]) {
            direction_ = 1;
            total_frames_ = 6;
            moving_ = true;
            if (can_move) {
                client->do_move(Direction::UP);
                last_move_time_ = frame_start_;
            }
        } else if (keys[SDL_SCANCODE_LEFT]) {
            direction_ = 2;
            total_frames_ = 5;
            moving_ = true;
            if (can_move) {
                client->do_move(Direction::LEFT);
                last_move_time_ = frame_start_;
            }
        } else if (keys[SDL_SCANCODE_RIGHT]) {
            direction_ = 3;
            total_frames_ = 5;
            moving_ = true;
            if (can_move) {
                client->do_move(Direction::RIGHT);
                last_move_time_ = frame_start_;
            }
        }
    }
}

// NUEVAS
void GameClient::send_chat_message(const std::string& text) {
    if (text.empty())
        return;
    client->do_chat(text);
}

void GameClient::toggle_chat() {
    chat_active_ = !chat_active_;
    if (!chat_active_) {
        chat_input_.clear();
    }
}

void GameClient::process_chat_input(const SDL_Event& event) {
    if (event.type == SDL_TEXTINPUT) {
        chat_input_ += event.text.text;
    } else if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_BACKSPACE && !chat_input_.empty()) {
            chat_input_.pop_back();
        } else if (event.key.keysym.sym == SDLK_RETURN && !chat_input_.empty()) {
            send_chat_message(chat_input_);
            chat_input_.clear();
        }
    }
}
