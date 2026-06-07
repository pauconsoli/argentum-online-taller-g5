#include "game_client.h"

#include <memory>
#include <stdexcept>

#include "client_map.h"
#include "common/updates/login_ok_update.h"
#include "common/updates/match_created_update.h"
#include "common/updates/match_joined_update.h"
#include "common/updates/moved_update.h"

GameClient::GameClient(int width, int height, const std::string& host, const std::string& port):
    window(nullptr),
    renderer(nullptr),
    hud(nullptr),
    sprite_manager(nullptr),
    client(host, port),
    camera(width, height),
    my_player_id(0),
    player_x(400),
    player_y(300),
    width(width),
    height(height) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error(SDL_GetError());
    }
    window = SDL_CreateWindow("Argentum Online - G5", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        SDL_Quit();
        throw std::runtime_error(SDL_GetError());
    }
    renderer = new Renderer(window);
    sprite_manager = new SpriteManager(renderer->get_sdl_renderer());
    sprite_manager->load("body", "client/assets/body.png");
    sprite_manager->load("head", "client/assets/head.png");
    sprite_manager->load_terrain_textures("client/assets");
    hud = new Hud(renderer->get_sdl_renderer(), "client/assets/font.ttf");

    client.start();
}

GameClient::~GameClient() {
    client.stop();
    client.join();
    delete hud;
    delete sprite_manager;
    delete renderer;
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void GameClient::run() {
    bool running = true;
    SDL_Event event;

    const int SPEED = 4;
    const int frame_w = 27;
    const int frame_h = 48;
    const int tile_w = 32;
    const int tile_h = 32;
    // head.png: stride 27px wide (matches body), 64px tall per direction row
    const int head_frame_w = 27;
    const int head_frame_h = 64;

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

        const Uint8* keys = SDL_GetKeyboardState(NULL);
        bool moving = false;

        if (keys[SDL_SCANCODE_DOWN]) {
            client.do_move(Direction::DOWN);
            player_y += SPEED;
            direction = 0;
            total_frames = 6;
            moving = true;
        } else if (keys[SDL_SCANCODE_UP]) {
            client.do_move(Direction::UP);
            player_y -= SPEED;
            direction = 1;
            total_frames = 6;
            moving = true;
        } else if (keys[SDL_SCANCODE_LEFT]) {
            client.do_move(Direction::LEFT);
            player_x -= SPEED;
            direction = 2;
            total_frames = 5;
            moving = true;
        } else if (keys[SDL_SCANCODE_RIGHT]) {
            client.do_move(Direction::RIGHT);
            player_x += SPEED;
            direction = 3;
            total_frames = 5;
            moving = true;
        }

        auto& update_queue = client.get_received_updates();
        std::unique_ptr<GameUpdate> update;
        while (update_queue.try_pop(update)) {}

        if (moving) {
            Uint32 now = SDL_GetTicks();
            if (now - last_frame_time > frame_delay) {
                current_frame = (current_frame + 1) % total_frames;
                last_frame_time = now;
            }
        } else {
            current_frame = 0;
        }

        int frame_x = current_frame * frame_w;
        int frame_y = direction * frame_h;
        int head_frame_y = direction * head_frame_h;

        // se centra la camara
        camera.center_on(player_x, player_y, client_map.get_width() * tile_w,
                         client_map.get_height() * tile_h);
        // convierte coordenadas del mundo a coordenadas de pantalla
        int screen_x = camera.get_screen_x(player_x);
        int screen_y = camera.get_screen_y(player_y);

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

        // Body
        renderer->draw_frame(sprite_manager->get("body"), frame_x, frame_y, frame_w, frame_h,
                             screen_x, screen_y);

        // Head: same x as body (both 27px wide), positioned so visible head
        // content (rows 16-37 of 64px frame) sits just above body content (row 17+)
        int const head_offset_x[] = {0, 0, 0, 0};
        int const head_offset_y[] = {-8, -15, -15, -15};
        // down, up, left, right

        int head_x = screen_x + head_offset_x[direction];
        int head_y = screen_y + head_offset_y[direction];

        renderer->draw_frame(sprite_manager->get("head"), current_frame * head_frame_w,
                             head_frame_y, head_frame_w, head_frame_h, head_x, head_y);

        hud->draw(100, 100, 50, 100, 1);
        renderer->present();

        Uint32 elapsed = SDL_GetTicks() - frame_start;
        if (elapsed < frame_time_ms) {
            SDL_Delay(frame_time_ms - elapsed);
        }
    }
}
