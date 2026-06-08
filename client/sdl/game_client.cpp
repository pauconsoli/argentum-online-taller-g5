#include "game_client.h"

#include <memory>
#include <stdexcept>

#include "client_map.h"
#include "common/updates/error_update.h"
#include "common/updates/login_ok_update.h"
#include "common/updates/match_created_update.h"
#include "common/updates/match_joined_update.h"
#include "common/updates/moved_update.h"
#include "common/updates/snapshot_update.h"
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

GameClient::GameClient(int width, int height, const std::string& host, const std::string& port):
    window(nullptr),
    renderer(nullptr),
    hud(nullptr),
    mini_chat(nullptr),
    sprite_manager(nullptr),
    client(host, port),
    camera(width, height),
    my_player_id(0),
    my_race(0),
    my_klass(0),
    player_x(400),
    player_y(300),
    width(width),
    height(height) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error(SDL_GetError());
    }
    my_hp = 100;
    my_max_hp = 100;
    my_mp = 100;
    my_max_mp = 100;
    my_level = 1;

    window = SDL_CreateWindow("Argentum Online - G5", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        SDL_Quit();
        throw std::runtime_error(SDL_GetError());
    }
    renderer = new Renderer(window);
    sprite_manager = new SpriteManager(renderer->get_sdl_renderer());
    sprite_manager->load_body_textures("client/assets");
    sprite_manager->load_terrain_textures("client/assets");
    hud = new Hud(renderer->get_sdl_renderer(), "client/assets/font.ttf", height, width);
    mini_chat = new MiniChat(renderer->get_sdl_renderer(), "client/assets/font.ttf", width);

    client.start();
}

GameClient::~GameClient() {
    client.stop();
    client.join();
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

    // direction: 0=down, 1=up, 2=left, 3=right (persists across frames)
    // TODO(chiaradelaurentis): esto viene del QT
    client.do_login("player1");
    {
        auto& q = client.get_received_updates();
        bool logged_in = false;
        bool in_match = false;
        uint32_t match_id = 0;

        // espera respuesta del servidor hasta que comience la partida
        while (running && !in_match) {
            // evita que la ventana quede congelada
            while (SDL_PollEvent(&event)) {
                running = input_handler.handle(event);
            }
            std::unique_ptr<GameUpdate> u;
            while (q.try_pop(u)) {
                // Cliente manda LOGIN
                // Servidor responde LOGIN_OK

                // Cliente manda CREATE_MATCH
                // Servidor responde MATCH_CREATED

                // Cliente manda JOIN_MATCH
                // Servidor responde MATCH_JOINED
                // esperar LOGIN_OK
                // crear sala1
                // esperar MATCH_CREATED
                // unirse a esa sala
                // esperar MATCH_JOINED
                // arrancar juego
                // ver que tipo de mensaje recibe
                switch (u->get_type()) {
                    case UpdateType::LOGIN_OK:
                        my_player_id =
                            static_cast<LoginOkUpdate&>(*u).player_id;  // que tipo de dato es este?
                        logged_in = true;
                        // PARTIDA HARDCODEADA
                        client.do_create_match("sala1", 4);
                        break;
                    case UpdateType::MATCH_CREATED:
                        match_id = static_cast<MatchCreatedUpdate&>(*u).match_id;
                        client.do_join_match(match_id);
                        break;
                    case UpdateType::MATCH_JOINED:
                        // este es el ultimo paso

                        in_match = true;
                        client.do_select_race_class(static_cast<uint8_t>(PlayerRace::HUMAN),
                                                    static_cast<uint8_t>(PlayerClass::WARRIOR));
                        break;

                    default:
                        break;
                }
            }
            (void) logged_in;
            SDL_Delay(10);
        }
    }

    if (!running)
        return;

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

    while (running) {
        Uint32 frame_start = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            running = input_handler.handle(event);
        }

        const Uint8* keys = SDL_GetKeyboardState(nullptr);
        bool moving = false;

        if (keys[SDL_SCANCODE_DOWN]) {
            client.do_move(Direction::DOWN);
            direction = 0;
            total_frames = 6;
            moving = true;
        } else if (keys[SDL_SCANCODE_UP]) {
            client.do_move(Direction::UP);
            direction = 1;
            total_frames = 6;
            moving = true;
        } else if (keys[SDL_SCANCODE_LEFT]) {
            client.do_move(Direction::LEFT);
            direction = 2;
            total_frames = 5;
            moving = true;
        } else if (keys[SDL_SCANCODE_RIGHT]) {
            client.do_move(Direction::RIGHT);
            direction = 3;
            total_frames = 5;
            moving = true;
        }

        auto& update_queue = client.get_received_updates();
        std::unique_ptr<GameUpdate> update;
        while (update_queue.try_pop(update)) {
            switch (update->get_type()) {
                case UpdateType::ERROR: {
                    const auto& eu = static_cast<const ErrorUpdate&>(*update);
                    mini_chat->add_message("Error: " + eu.detail);
                    break;
                }
                case UpdateType::SNAPSHOT: {
                    const auto& snap = static_cast<const SnapshotUpdate&>(*update);
                    for (const auto& ps : snap.players) {
                        players[ps.player_id] = ps;
                        if (ps.player_id == my_player_id) {
                            player_x = ps.x;
                            player_y = ps.y;
                            my_race = ps.race;
                            my_klass = ps.klass;
                            my_hp = ps.hp;
                            my_mp = ps.mp;
                            my_max_hp = ps.max_hp;
                            my_max_mp = ps.max_mp;
                            my_level = ps.level;
                        }
                    }
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
                TerrainType terrain = (col >= 0 && col < client_map.get_width() && row >= 0 &&
                                       row < client_map.get_height()) ?
                                          client_map.at(col, row).terrain :
                                          TerrainType::GRASS;
                renderer->draw_frame(sprite_manager->get_terrain(terrain), 0, 0, tile_w, tile_h, tx,
                                     ty);
            }
        }

        // Dibuja todos los jugadores con el body según raza/clase del snapshot
        static const int head_offset_x[] = {0, 0, 0, 0};
        static const int head_offset_y[] = {-8, -15, -15, -15};

        for (const auto& [pid, ps] : players) {
            int px = camera.get_screen_x(ps.x);
            int py = camera.get_screen_y(ps.y);

            int p_dir = (pid == my_player_id) ? direction : 0;
            int p_frame = (pid == my_player_id) ? current_frame : 0;
            int p_total = (p_dir < 2) ? 6 : 5;
            int p_frame_clamped = p_frame % p_total;

            int p_frame_x = p_frame_clamped * frame_w;
            int p_frame_y = p_dir * frame_h;

            renderer->draw_frame(sprite_manager->get_body(ps.race, ps.klass), p_frame_x, p_frame_y,
                                 frame_w, frame_h, px, py);

            int hx = px + head_offset_x[p_dir];
            int hy = py + head_offset_y[p_dir];
            // nueva cabeza: src x=0 (sin frames), y = dirección * 64
            renderer->draw_frame(sprite_manager->get_head(head_index_for_race(ps.race)), 0,
                                 p_dir * head_h, head_w, head_h, hx, hy);
        }

        hud->draw(my_hp, my_max_hp, my_mp, my_max_mp, my_level);
        hud->draw_inventory(sprite_manager);

        mini_chat->draw();
        renderer->present();

        Uint32 elapsed = SDL_GetTicks() - frame_start;
        if (elapsed < frame_time_ms) {
            SDL_Delay(frame_time_ms - elapsed);
        }
    }
}
