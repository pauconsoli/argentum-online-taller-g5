#include "game_client.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
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

static ItemType get_item_type(const std::string& name) {
    static const std::unordered_map<std::string, ItemType> table = {
        {"Espada", ItemType::WEAPON},
        {"Hacha", ItemType::WEAPON},
        {"Martillo", ItemType::WEAPON},
        {"Arco simple", ItemType::WEAPON},
        {"Arco compuesto", ItemType::WEAPON},
        {"Vara de fresno", ItemType::STAFF},
        {"Báculo nudoso", ItemType::STAFF},
        {"Báculo engarzado", ItemType::STAFF},
        {"Flauta élfica", ItemType::STAFF},
        {"Armadura de cuero", ItemType::ARMOR},
        {"Armadura de placas", ItemType::ARMOR},
        {"Túnica azul", ItemType::ARMOR},
        {"Capucha", ItemType::HELMET},
        {"Casco de hierro", ItemType::HELMET},
        {"Sombrero mágico", ItemType::HELMET},
        {"Escudo de tortuga", ItemType::SHIELD},
        {"Escudo de hierro", ItemType::SHIELD},
    };
    auto it = table.find(name);
    return it != table.end() ? it->second : ItemType::OTHER;
}

// esto deberia estar en sprite manager??
static std::string get_base_asset_dir() {
    if (const char* env_dir = std::getenv("ARGENTUM_DATA_DIR")) {
        return std::string(env_dir);
    }
    return "assets";
}

// esto deberia estar aca? o donde?
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
    audio_manager(nullptr),
    client(std::make_unique<Client>(host, port)),
    camera(width, height),
    my_player_id(1),  // ID provisorio/dummy para standalone
    my_race(1),
    my_klass(1),
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
    std::string base_assets = get_base_asset_dir();
    std::string font_path = base_assets + "/fonts/font.ttf";
    sprite_manager = new SpriteManager(renderer->get_sdl_renderer());
    sprite_manager->load_body_textures(base_assets);
    sprite_manager->load_terrain_textures(base_assets);
    terrain_renderer_ = new TerrainRenderer(renderer, sprite_manager, camera);
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

        render_players(config_.tile_width, config_.tile_height, direction_, current_frame_);
        render_npcs(config_.tile_width, config_.tile_height);

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

            case UpdateType::ERROR: {
                // Filtra errores de movimiento (silenciosos) y traduce el resto al chat.
                const auto& eu = static_cast<const ErrorUpdate&>(*update);
                const auto& d = eu.detail;
                if (d.find("move_player") != std::string::npos ||
                    d.find("mover") != std::string::npos) {
                    break;
                }
                if (d.find("muertos") != std::string::npos ||
                    d.find("muerto") != std::string::npos || d.find("ghost") != std::string::npos ||
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
                // Reemplaza el estado completo del mundo: jugadores, NPCs e items del suelo.
                const auto& snap = static_cast<const SnapshotUpdate&>(*update);
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

                // Actualizar estado de animación desde los datos de red.
                // La conversión de dirección es obligatoria: el servidor manda
                // UP=0,DOWN=1,LEFT=2,RIGHT=3 pero el spritesheet tiene filas en orden
                // south=0,north=1,west=2,east=3.
                static const uint8_t kDirToRow[] = {1, 0, 2, 3};
                for (const auto& ns : snap.npcs) {
                    auto& anim = npc_anim_states_[ns.npc_id];
                    NPCVisualType new_type = npc_visual_type_from_network(ns.npc_type);
                    if (anim.sprite_type != new_type) {
                        if (new_type == NPCVisualType::UNKNOWN)
                            std::cerr << "[NPC] id=" << ns.npc_id
                                      << " tipo desconocido=" << ns.npc_type << "\n";
                        else
                            std::cerr << "[NPC] id=" << ns.npc_id << " tipo=" << ns.npc_type
                                      << " dir=" << static_cast<int>(ns.direction)
                                      << " moving=" << ns.is_moving << "\n";
                        anim.sprite_type = new_type;
                        anim.current_frame = 0;
                    }
                    anim.direction = (ns.direction < 4) ? kDirToRow[ns.direction] : 0;
                    anim.is_moving = ns.is_moving;
                }

                // Limpia estados de animación de NPCs que ya no existen en el snapshot.
                for (auto it = npc_anim_states_.begin(); it != npc_anim_states_.end();) {
                    if (npcs_.find(it->first) == npcs_.end())
                        it = npc_anim_states_.erase(it);
                    else
                        ++it;
                }
                break;
            }
            case UpdateType::PLAYER_LEFT: {
                // Un jugador se desconectó: lo elimina del mapa local.
                const auto& pu = static_cast<const PlayerLeftUpdate&>(*update);
                players.erase(pu.player_id);
                break;
            }
            case UpdateType::WORLD_MAP: {
                // El servidor mandó el mapa completo; lo reemplaza en el cliente.
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
                // Actualiza el inventario y el oro del jugador local.
                const auto& iu = static_cast<const InventoryUpdate&>(*update);
                inventory_slots_ = iu.get_items();
                my_gold = iu.get_gold();
                break;
            }
            case UpdateType::ATTACKED: {
                // Muestra el resultado del ataque en el chat y reproduce el sonido correcto.
                const auto& au = static_cast<const AttackUpdate&>(*update);
                const AttackResult& r = au.get_result();
                if (r.evaded) {
                    mini_chat->add_message("Ataque esquivado");
                } else if (r.attacker_id == my_player_id) {
                    std::string target_name = "";
                    auto player_it = players.find(r.target_id);
                    if (player_it != players.end()) {
                        target_name = player_it->second.nick;
                    } else {
                        auto npc_it = npcs_.find(r.target_id);
                        if (npc_it != npcs_.end()) {
                            target_name = npc_it->second.name;
                        }
                    }

                    if (r.is_healing) {
                        mini_chat->add_message(r.weapon_or_spell_name + ": curaste a " +
                                               target_name + " por " +
                                               std::to_string(r.heal_amount) + " puntos");
                    } else if (!r.target_died) {
                        mini_chat->add_message(r.weapon_or_spell_name + ": causaste " +
                                               std::to_string(r.damage) + " de daño a " +
                                               target_name);
                    }
                } else if (r.target_id == my_player_id) {
                    if (r.is_healing) {
                        std::string healer_name = "";
                        auto player_it = players.find(r.attacker_id);
                        if (player_it != players.end()) {
                            healer_name = player_it->second.nick;
                        } else {
                            auto npc_it = npcs_.find(r.attacker_id);
                            if (npc_it != npcs_.end()) {
                                healer_name = npc_it->second.name;
                            }
                        }
                        mini_chat->add_message(healer_name + " te curó " +
                                               std::to_string(r.heal_amount) + " puntos de vida");
                    } else if (!r.target_died) {
                        mini_chat->add_message("Recibiste " + std::to_string(r.damage) +
                                               " de daño");
                    }
                }

                switch (r.type) {
                    case AttackType::NORMAL:
                        if (!r.evaded && r.damage > 0)
                            audio_manager->play_sound("melee_hit");
                        break;
                    case AttackType::RANGED:
                        audio_manager->play_sound("ranged_attack");
                        break;
                    case AttackType::MAGIC:
                        if (r.is_healing) {
                            // heal.wav no existe en assets; curación sin audio por ahora
                        } else {
                            std::string lname = r.weapon_or_spell_name;
                            std::transform(
                                lname.begin(), lname.end(), lname.begin(),
                                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                            if (lname.find("explos") != std::string::npos)
                                audio_manager->play_sound("explosion");
                            // magic_attack.wav no existe; otros hechizos sin audio por ahora
                        }
                        break;
                }
                break;
            }
            case UpdateType::DEATH: {
                // Muerte propia: avisa al jugador. Muerte ajena: sonido atenuado por distancia.
                const auto& du = static_cast<const DeathUpdate&>(*update);
                if (du.get_dead_id() == my_player_id) {
                    mini_chat->add_message("Moriste. Dirigite al sacerdote para resucitar");
                    audio_manager->play_sound("death");
                } else {
                    int vol = MIX_MAX_VOLUME;
                    uint32_t dead_id = du.get_dead_id();
                    int dead_tx = -1, dead_ty = -1;
                    auto pit = players.find(dead_id);
                    if (pit != players.end()) {
                        const auto& dead_player_snapshot = pit->second;
                        dead_tx = dead_player_snapshot.x;
                        dead_ty = dead_player_snapshot.y;
                        if (du.get_killer_id() == my_player_id) {
                            mini_chat->add_message("Mataste a " + dead_player_snapshot.nick);
                        } else {
                            mini_chat->add_message("Un jugador murio en combate");
                        }
                    } else {
                        auto nit = npcs_.find(dead_id);
                        if (nit != npcs_.end()) {
                            const auto& dead_npc_snapshot = nit->second;
                            dead_tx = dead_npc_snapshot.x;
                            dead_ty = dead_npc_snapshot.y;
                            if (du.get_killer_id() == my_player_id) {
                                mini_chat->add_message("Mataste a " + dead_npc_snapshot.name);
                            } else {
                                mini_chat->add_message("Un NPC murió en combate");
                            }
                        }
                    }
                    if (dead_tx >= 0) {
                        int dx = dead_tx - player_x / tile_w;
                        int dy = dead_ty - player_y / tile_h;
                        float dist = std::sqrt(static_cast<float>(dx * dx + dy * dy));
                        constexpr float kMaxDist = 20.0f;
                        vol = (dist >= kMaxDist) ?
                                  0 :
                                  static_cast<int>((1.0f - dist / kMaxDist) * MIX_MAX_VOLUME);
                    }
                    audio_manager->play_sound("death", vol);
                }
                break;
            }
            case UpdateType::CHAT_MSG: {
                // Mensaje de chat recibido del servidor: lo muestra en el mini_chat.
                // El update es ChatMsgUpdate (sender_nick + text), no SystemMsgUpdate.
                const auto& message = static_cast<const ChatMsgUpdate&>(*update);
                mini_chat->add_message(message.sender_nick + ": " + message.text);
                break;
            }
            default:
                break;
        }
    }
}


// Dibuja todos los jugadores: cuerpo animado + cabeza + items equipados visibles + nickname.
void GameClient::render_players(int tile_w, int tile_h, int direction, int current_frame) {

    // Itera sobre todos los jugadores conocidos (pid = id, ps = estado del jugador).
    for (const auto& [pid, ps] : players) {

        // Convierte la posición en tiles a coordenadas de pantalla relativas a la cámara.
        int px = camera.get_screen_x(ps.x * tile_w);
        int py = camera.get_screen_y(ps.y * tile_h);

        // Obtiene la textura del cuerpo según la raza y clase del jugador.

        SDL_Texture* body_texture = sprite_manager->get_body(ps.race, ps.klass);
        // Obtiene la textura de la cabeza según el índice de sprite que corresponde a su raza.
        SDL_Texture* head_texture = sprite_manager->get_head(head_index_for_race(ps.race));

        // Solo el jugador local usa la dirección real; los remotos miran hacia abajo (dirección 0).
        int player_direction = (pid == my_player_id) ? direction : 0;

        // Solo el jugador local avanza frames de animación; los remotos se fijan en frame 0.
        int player_frame = (pid == my_player_id) ? current_frame : 0;

        // Dibuja cuerpo + cabeza en pantalla; is_ghost afecta la opacidad del render.
        character_renderer->draw_character(body_texture, head_texture, px, py, player_direction,
                                           player_frame, ps.is_ghost);

        // Los items equipados y el nickname solo se renderizan para el jugador local.
        if (pid == my_player_id) {
            // Recorre todos los slots del inventario buscando items que estén equipados.
            for (const auto& islot : inventory_slots_) {
                // Salta slots vacíos o sin item equipado.
                if (!islot.is_equipped || islot.item_name.empty())
                    continue;

                // Determina el tipo de item (arma, escudo, casco…) a partir de su nombre.
                ItemType itype = get_item_type(islot.item_name);

                // Obtiene la textura SDL del sprite del item por nombre.
                SDL_Texture* itex =
                    sprite_manager->get_item(SpriteManager::item_key_for_name(islot.item_name));
                // Si no hay textura cargada para este item, lo omite.
                if (!itex)
                    continue;
                // Superpone el sprite del item equipado sobre el personaje.
                character_renderer->draw_equipped_item(px, py, itex, itype, tile_w);
            }
            // Dibuja el nickname centrado, un poco por encima de la cabeza del jugador.
        }
        character_renderer->draw_nickname(ps.nick, px, py);
    }
}

struct NPCSpriteInfo {
    int fw;
    int fh;
    int fpd[4];
    int draw_w = 0;
    int draw_h = 0;
    int head_index = 0;
};

static NPCSpriteInfo npc_sprite_info(NPCVisualType t) {
    switch (t) {
        case NPCVisualType::BANKER:
            return {26, 46, {7, 7, 7, 7}};
        case NPCVisualType::PRIEST:
            return {27, 47, {6, 6, 5, 5}, 0, 0, 3};
        case NPCVisualType::MERCHANT:
            return {27, 47, {6, 6, 5, 5}, 0, 0, 30};
        case NPCVisualType::GOBLIN:
            return {32, 32, {6, 6, 6, 6}};
        case NPCVisualType::SKELETON:
            return {25, 52, {6, 6, 5, 5}};
        case NPCVisualType::ZOMBIE:
            return {128, 128, {6, 6, 6, 4}};
        case NPCVisualType::SPIDER:
            return {128, 128, {8, 8, 8, 8}};
        case NPCVisualType::ORC:
            return {24, 52, {6, 6, 5, 5}};
        case NPCVisualType::GOLEM_ICE:
            return {128, 128, {8, 8, 8, 8}, 64, 64};
        case NPCVisualType::GOLEM_STONE:
            return {128, 128, {8, 8, 8, 8}, 64, 64};
        case NPCVisualType::GOLEM_INFERNAL:
            return {128, 128, {8, 8, 8, 8}, 64, 64};
        default:
            return {32, 32, {1, 1, 1, 1}};
    }
}

// Dibuja todos los NPCs con su animación correspondiente; los humanoides también tienen cabeza.
void GameClient::render_npcs(int tile_w, int tile_h) {
    // cambia cada 150
    const Uint32 npc_frame_delay = 150;
    for (auto& [nid, ns] : npcs_) {
        // nid es el id del npc, ns es su estado (posicion, tipo, direccion, etc)

        // y anim es el estado de animacion local (frame actual, ultima vez que cambio de frame, etc)
        auto& anim = npc_anim_states_[nid];

        // si no existe, lo crea
        SDL_Texture* npc_tex = sprite_manager->get_npc(anim.sprite_type);
        if (!npc_tex)
            continue;
        NPCSpriteInfo info = npc_sprite_info(anim.sprite_type);
        int dir = static_cast<int>(anim.direction);

        if (dir > 3)
            dir = 0;
        int max_frames = info.fpd[dir];

        // Avanza la animación sólo si el NPC está en movimiento.
        if (anim.is_moving) {
            Uint32 now = SDL_GetTicks();
            if (now - anim.last_frame_time > npc_frame_delay) {
                anim.current_frame = (anim.current_frame + 1) % max_frames;
                anim.last_frame_time = now;
            }
        } else {
            anim.current_frame = 0;
        }

        int frame = anim.current_frame % max_frames;
        int px = camera.get_screen_x(ns.x * tile_w);
        int py = camera.get_screen_y(ns.y * tile_h);
        int dw = (info.draw_w > 0) ? info.draw_w : info.fw;
        int dh = (info.draw_h > 0) ? info.draw_h : info.fh;
        int body_x = px + (tile_w - dw) / 2;
        int body_y = py + tile_h - dh;
        renderer->draw_frame_scaled(npc_tex, frame * info.fw, dir * info.fh, info.fw, info.fh,
                                    body_x, body_y, dw, dh);

        // NPCs humanoides (banquero, sacerdote, mercader) tienen cabeza separada.
        if (info.head_index > 0) {
            static const int body_row_to_head_row[] = {2, 0, 3, 1};
            static const int head_y_offset[] = {-13, -19, -18, -17};
            constexpr int head_w = 27;
            constexpr int head_h = 64;
            SDL_Texture* head_tex = sprite_manager->get_head(info.head_index);
            if (head_tex) {
                renderer->draw_frame(head_tex, 0, body_row_to_head_row[dir] * head_h, head_w,
                                     head_h, body_x, body_y + head_y_offset[dir]);
            }
        }
    }
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
                            mini_chat->add_message("Seleccionaste un NPC");
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
