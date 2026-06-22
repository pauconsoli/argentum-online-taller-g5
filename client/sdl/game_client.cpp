// VERSION ACTUAL SIN CAMBIOS DE PAU

#include "game_client.h"

#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include "client_map.h"
#include "common/cheat_type.h"
#include "common/clan/clan_action.h"
#include "world_renderer.h"

// esto deberia estar en sprite manager??
static std::string get_base_asset_dir() {
    if (const char* env_dir = std::getenv("ARGENTUM_DATA_DIR")) {
        return std::string(env_dir);
    }
    return "assets";
}


// Constructor standalone
GameClient::GameClient(int width, int height, bool fullscreen, const std::string& host,
                       const std::string& port):
    client(std::make_unique<Client>(host, port)),
    camera(width, height),
    width(width),
    height(height),
    state(1, 1, 1),
    update_handler(state, *this) {
    init_subsystems(fullscreen, false);
}

GameClient::GameClient(int width, int height, bool fullscreen, std::unique_ptr<Client> c,
                       uint8_t race, uint8_t klass, uint32_t player_id):
    config(SdlConfig::load("client/config/sdl_config.toml")),
    client(std::move(c)),
    camera(width, height),
    width(width),
    height(height),
    state(player_id, race, klass),
    update_handler(state, *this) {
    init_subsystems(fullscreen, true);
}

GameClient::~GameClient() {
    client->stop();
    client->join();
    // El resto del cleanup ocurre automáticamente por destrucción de unique_ptrs
    // en orden inverso de declaración: renderers → sprite_manager → HUD/chat →
    // character_renderer → audio_manager → renderer → window (SDL_DestroyWindow
    // + SDL_Quit via SdlWindowDeleter).
}

void GameClient::message(const std::string& text) {
    mini_chat->add_message(text);
}

void GameClient::play(const std::string& sound, int vol) {
    if (vol < 0)
        audio_manager->play_sound(sound);
    else
        audio_manager->play_sound(sound, vol);
}

void GameClient::show_clan_review(const ClanReviewUpdate& cru) {
    clan_panel->set_data(cru);
}

void GameClient::init_subsystems(bool fullscreen, bool load_font) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
        throw std::runtime_error(SDL_GetError());
    Uint32 flags = SDL_WINDOW_SHOWN;
    if (fullscreen)
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    window.reset(SDL_CreateWindow("Argentum Online - G5", SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED, width, height, flags));
    if (!window) {
        SDL_Quit();
        throw std::runtime_error(SDL_GetError());
    }
    renderer = std::make_unique<Renderer>(window.get());
    SDL_RenderSetLogicalSize(renderer->get_sdl_renderer(), width, height);
    character_renderer = std::make_unique<CharacterRenderer>(renderer.get());
    std::string base_assets = get_base_asset_dir();
    std::string font_path = base_assets + "/fonts/font.ttf";
    if (load_font)
        renderer->load_font(font_path, 12);
    sprite_manager = std::make_unique<SpriteManager>(renderer->get_sdl_renderer());
    sprite_manager->load_body_textures(base_assets);
    sprite_manager->load_terrain_textures(base_assets);
    terrain_renderer =
        std::make_unique<TerrainRenderer>(renderer.get(), sprite_manager.get(), camera);
    npc_renderer = std::make_unique<NPCRenderer>(renderer.get(), sprite_manager.get(), camera);
    player_renderer =
        std::make_unique<PlayerRenderer>(character_renderer.get(), sprite_manager.get(), camera);
    hud = std::make_unique<Hud>(renderer->get_sdl_renderer(), font_path, height, width);
    mini_chat = std::make_unique<MiniChat>(renderer->get_sdl_renderer(), font_path, width);
    help_menu = std::make_unique<HelpMenu>(renderer->get_sdl_renderer(), font_path, width, height);
    clan_panel =
        std::make_unique<ClanPanel>(renderer->get_sdl_renderer(), font_path, width, height);
    audio_manager = std::make_unique<AudioManager>();
    load_audio_assets();
    world_renderer = std::make_unique<WorldRenderer>(
        *renderer, *sprite_manager, *terrain_renderer, *npc_renderer, *player_renderer, *hud,
        *mini_chat, *help_menu, *clan_panel, camera, config);
}

void GameClient::run() {
    running = true;

    audio_manager->play_background_music(get_base_asset_dir() + "/audio/music/background.mp3",
                                         MIX_MAX_VOLUME / 2);

    ClientMap client_map = build_sample_client_map();

    state.reset_position();

    direction = 0;
    current_frame = 0;
    total_frames = 6;
    last_frame_time = SDL_GetTicks();
    last_move_time = 0;

    while (running) {
        frame_start = SDL_GetTicks();

        process_sdl_events();
        process_keyword_input();
        update_handler.apply_pending(client->get_received_updates(), client_map, config.tile_width,
                                     config.tile_height);
        npc_renderer->sync_from_snapshot(state.last_npc_snapshot());


        if (moving) {
            Uint32 now = SDL_GetTicks();
            if (now - last_frame_time > config.frame_delay_ms) {
                current_frame = (current_frame + 1) % total_frames;
                last_frame_time = now;
            }
        } else {
            current_frame = 0;
        }


        if (state.player_x() >= 0 && state.player_y() >= 0) {
            camera.center_on(state.player_x(), state.player_y(),
                             client_map.get_width() * config.tile_width,
                             client_map.get_height() * config.tile_height);
        }

        world_renderer->draw(state, client_map, direction, current_frame, chat_active, chat_input,
                             music_paused, width, height);

        Uint32 elapsed = SDL_GetTicks() - frame_start;
        if (elapsed < config.frame_time_ms()) {
            SDL_Delay(config.frame_time_ms() - elapsed);
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
    while (SDL_PollEvent(&event)) {
        if (!input_handler.handle_quit(event))
            running = false;

        if (clan_panel->is_visible() &&
            ((event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) ||
             (event.type == SDL_MOUSEBUTTONDOWN))) {
            clan_panel->hide();
            continue;
        }
        if (chat_active) {
            handle_chat_event(event);
            continue;
        }
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) {
            chat_active = true;
            SDL_StartTextInput();
            continue;
        } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_h) {
            help_menu->toggle();
            continue;
        } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_m) {
            audio_manager->toggle_music();
            music_paused = audio_manager->is_music_paused();
            continue;
        } else if (event.type == SDL_KEYDOWN && (event.key.keysym.mod & KMOD_CTRL) &&
                   event.key.repeat == 0) {
            // cheats: Ctrl + numero, para probar sin NPCs
            switch (event.key.keysym.sym) {
                case SDLK_1:
                    client->do_cheat(CheatType::HEAL_FULL);
                    break;
                case SDLK_2:
                    client->do_cheat(CheatType::RESTORE_MANA);
                    break;
                case SDLK_3:
                    client->do_cheat(CheatType::DIE);
                    break;
                case SDLK_4:
                    client->do_cheat(CheatType::LEVEL_UP);
                    break;
                case SDLK_5:
                    client->do_cheat(CheatType::GIVE_GOLD);
                    break;
                default:
                    break;
            }
            continue;
        }
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            handle_left_click(event.button.x, event.button.y);
            continue;
        }
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT)
            handle_right_click(event.button.x, event.button.y);
    }
}

void GameClient::handle_chat_event(const SDL_Event& e) {
    if (e.type == SDL_TEXTINPUT) {
        chat_input += e.text.text;
        return;
    }
    if (e.type != SDL_KEYDOWN)
        return;
    switch (e.key.keysym.sym) {
        case SDLK_RETURN:
        case SDLK_RETURN2:
            handle_chat_submit();
            break;
        case SDLK_BACKSPACE:
            if (!chat_input.empty())
                chat_input.pop_back();
            break;
        case SDLK_ESCAPE:
            chat_input.clear();
            chat_active = false;
            SDL_StopTextInput();
            break;
        default:
            break;
    }
}

void GameClient::handle_chat_submit() {
    if (!chat_input.empty())
        dispatch_chat_command(chat_input);
    chat_input.clear();
    chat_active = false;
    SDL_StopTextInput();
}

static bool starts_with(const std::string& input, const std::string& prefix) {
    return input.rfind(prefix, 0) == 0;
}

static std::string arg_of(const std::string& input, const std::string& prefix) {
    return input.substr(prefix.size());
}

void GameClient::dispatch_chat_command(const std::string& input) {
    if (input == "/meditar") {
        client->do_meditate();
        return;
    }
    if (starts_with(input, "/tomar")) {
        client->do_pick_up();
        return;
    }
    if (starts_with(input, "/tirar")) {
        handle_drop_command(input);
        return;
    }
    if (input == "/curar") {
        handle_npc_command(NPCInteraction::HEAL, "", 0, input);
        return;
    }
    if (input == "/resucitar") {
        if (!state.is_ghost()) {
            mini_chat->add_message("No es posible resucitar. Estás vivo");
            return;
        }
        if (state.is_ghost() && state.selected_npc_id() == 0) {
            client->do_resurrect();
        } else {
            handle_npc_command(NPCInteraction::RESURRECT, "", 0, input);
        }
        return;
    }
    if (input == "/listar") {
        handle_npc_command(NPCInteraction::LIST, "", 0, input);
        return;
    }
    if (starts_with(input, "/comprar ")) {
        handle_npc_command(NPCInteraction::BUY, arg_of(input, "/comprar "), 0, input);
        return;
    }
    if (starts_with(input, "/vender ")) {
        handle_npc_command(NPCInteraction::SELL, arg_of(input, "/vender "), 0, input);
        return;
    }
    if (starts_with(input, "/depositar ")) {
        std::string rest = arg_of(input, "/depositar ");
        if (starts_with(rest, "oro") && (rest.size() == 3 || rest[3] == ' ')) {
            std::string num_str = rest.size() > 4 ? arg_of(rest, "oro ") : "";
            try {
                handle_npc_command(NPCInteraction::DEPOSIT_GOLD, "", std::stoi(num_str), input);
            } catch (...) {
                mini_chat->add_message("Cantidad inválida");
            }
        } else {
            handle_npc_command(NPCInteraction::DEPOSIT_ITEM, rest, 0, input);
        }
        return;
    }
    if (starts_with(input, "/retirar ")) {
        std::string rest = arg_of(input, "/retirar ");
        if (starts_with(rest, "oro") && (rest.size() == 3 || rest[3] == ' ')) {
            std::string num_str = rest.size() > 4 ? arg_of(rest, "oro ") : "";
            try {
                handle_npc_command(NPCInteraction::WITHDRAW_GOLD, "", std::stoi(num_str), input);
            } catch (...) {
                mini_chat->add_message("Cantidad inválida");
            }
        } else {
            handle_npc_command(NPCInteraction::WITHDRAW_ITEM, rest, 0, input);
        }
        return;
    }
    if (starts_with(input, "/fundar-clan ")) {
        std::string arg = arg_of(input, "/fundar-clan ");
        if (arg.empty()) {
            mini_chat->add_message("Uso: /fundar-clan <nombre>");
            return;
        }
        client->do_clan_action(ClanAction::FOUND, arg);
        return;
    }
    if (starts_with(input, "/unirse ")) {
        std::string arg = arg_of(input, "/unirse ");
        if (arg.empty()) {
            mini_chat->add_message("Uso: /unirse <nombre-del-clan>");
            return;
        }
        client->do_clan_action(ClanAction::JOIN_REQUEST, arg);
        return;
    }
    if (input == "/revisar-clan") {
        client->do_clan_action(ClanAction::REVIEW, "");
        return;
    }
    if (starts_with(input, "/clan-aceptar ")) {
        std::string arg = arg_of(input, "/clan-aceptar ");
        if (arg.empty()) {
            mini_chat->add_message("Uso: /clan-aceptar <nick>");
            return;
        }
        client->do_clan_action(ClanAction::ACCEPT, arg);
        return;
    }
    if (starts_with(input, "/clan-rechazar ")) {
        std::string arg = arg_of(input, "/clan-rechazar ");
        if (arg.empty()) {
            mini_chat->add_message("Uso: /clan-rechazar <nick>");
            return;
        }
        client->do_clan_action(ClanAction::REJECT, arg);
        return;
    }
    if (starts_with(input, "/clan-ban ")) {
        std::string arg = arg_of(input, "/clan-ban ");
        if (arg.empty()) {
            mini_chat->add_message("Uso: /clan-ban <nick>");
            return;
        }
        client->do_clan_action(ClanAction::BAN, arg);
        return;
    }
    if (starts_with(input, "/clan-kick ")) {
        std::string arg = arg_of(input, "/clan-kick ");
        if (arg.empty()) {
            mini_chat->add_message("Uso: /clan-kick <nick>");
            return;
        }
        client->do_clan_action(ClanAction::KICK, arg);
        return;
    }
    if (input == "/dejar-clan") {
        client->do_clan_action(ClanAction::LEAVE, "");
        return;
    }
    if (!input.empty() && input[0] == '/') {
        mini_chat->add_message("Comando desconocido");
        return;
    }
    if (!input.empty() && input[0] == '@') {
        size_t space_pos = input.find(' ');
        if (space_pos != std::string::npos && space_pos > 1) {
            std::string target_nick = input.substr(1, space_pos - 1);
            std::string msg = input.substr(space_pos + 1);
            client->do_private_chat(target_nick, msg);
            return;
        }
    }
    client->do_chat(input);
}

void GameClient::handle_npc_command(NPCInteraction type, const std::string& arg, int32_t amount,
                                    const std::string& /*display*/) {
    if (state.selected_npc_id() == 0) {
        mini_chat->add_message("Seleccioná un NPC primero");
        return;
    }
    client->do_interact(state.selected_npc_id(), type, arg, amount);
}

void GameClient::handle_drop_command(const std::string& input) {
    std::string rest = input.substr(6);
    size_t pos = rest.find_first_not_of(' ');
    int slot = -1;
    if (pos != std::string::npos) {
        try {
            slot = std::stoi(rest.substr(pos));
        } catch (...) {
            slot = -1;
        }
    } else {
        slot = state.selected_slot();
    }
    if (slot >= 0 && slot < static_cast<int>(state.inventory_slots().size()) &&
        !state.inventory_slots()[slot].item_name.empty()) {
        client->do_drop_item(static_cast<uint8_t>(slot));
        mini_chat->add_message("Tiraste el item");
        state.clear_slot_selection();
        return;
    }
    if (pos == std::string::npos && state.selected_slot() < 0) {
        mini_chat->add_message("Seleccioná un item con click derecho primero");
        return;
    }
    mini_chat->add_message("Slot inválido o vacío");
}

void GameClient::handle_left_click(int screen_x, int screen_y) {
    SDL_Rect music_btn = hud->get_music_button_rect();
    if (screen_x >= music_btn.x && screen_x < music_btn.x + music_btn.w &&
        screen_y >= music_btn.y && screen_y < music_btn.y + music_btn.h) {
        audio_manager->toggle_music();
        music_paused = audio_manager->is_music_paused();
        return;
    }
    int tile_x = (camera.get_x() + screen_x) / config.tile_width;
    int tile_y = (camera.get_y() + screen_y) / config.tile_height;
    try_attack_at_tile(tile_x, tile_y);
    if (!state.is_ghost())
        try_equip_at(screen_x, screen_y);
}

bool GameClient::try_attack_at_tile(int tile_x, int tile_y) {
    if (!state.is_ghost()) {
        for (const auto& [pid, ps] : state.players()) {
            if (pid == state.player_id() || ps.x != tile_x || ps.y != tile_y)
                continue;
            client->do_attack(pid);
            return true;
        }
    }
    for (const auto& [nid, ns] : state.npcs()) {
        if (ns.x != tile_x || ns.y != tile_y)
            continue;
        if (ns.is_hostile) {
            if (!state.is_ghost())
                client->do_attack(nid);
            return true;
        }
        state.select_npc(nid);
        switch (npc_visual_type_from_network(ns.npc_type)) {
            case NPCVisualType::PRIEST:
                mini_chat->add_message("Seleccionaste al sacerdote");
                break;
            case NPCVisualType::MERCHANT:
                mini_chat->add_message("Seleccionaste al comerciante");
                break;
            case NPCVisualType::BANKER:
                mini_chat->add_message("Seleccionaste al banquero");
                break;
            default:
                mini_chat->add_message("Seleccionaste a un NPC");
                break;
        }
        return true;
    }
    return false;
}

void GameClient::try_equip_at(int screen_x, int screen_y) {
    int slot = hud->get_slot_at(screen_x, screen_y);
    if (slot < 0 || slot >= static_cast<int>(state.inventory_slots().size()))
        return;
    if (state.inventory_slots()[slot].item_name.empty())
        return;
    client->do_equip_item(static_cast<uint8_t>(slot));
}

void GameClient::handle_right_click(int screen_x, int screen_y) {
    int slot = hud->get_slot_at(screen_x, screen_y);
    if (slot < 0 || slot >= static_cast<int>(state.inventory_slots().size()))
        return;
    if (state.inventory_slots()[slot].item_name.empty())
        return;
    state.select_slot(slot);
}

void GameClient::process_keyword_input() {
    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    moving = false;
    if (!chat_active) {
        bool can_move = (frame_start - last_move_time >= config.move_interval_ms);
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
}
