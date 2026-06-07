#include "game_client.h"

#include <memory>
#include <stdexcept>

#include "common/updates/login_ok_update.h"
#include "common/updates/match_created_update.h"
#include "common/updates/match_joined_update.h"
#include "common/updates/move_update.h"

GameClient::GameClient(int width, int height, const std::string& host, const std::string& port):
    // se inicializan el null para evitar que tengan una direccion basura
    window(nullptr),
    renderer(nullptr),
    hud(nullptr),
    sprite_manager(nullptr),
    client(host, port),
    camera(width, height),
    // lo inciializamos con 0 para que tenga un valor valido, verificar porque player_id 0 podria
    // ser un id valido
    my_player_id(0),
    player_x(0),
    player_y(0),
    width(width),
    height(height) {

    // sdl init video habilita ventanas, renderizado, pantalla, etc
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error(SDL_GetError());
    }

    // se crea la ventana centrada, con el alto y ancho que necesitamos
    window = SDL_CreateWindow("Argentum Online - G5", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (window == nullptr) {  // si falla, el puntero queda en nulo
        SDL_Quit();
        throw std::runtime_error(SDL_GetError());
    }
    renderer = new Renderer(window);
    sprite_manager = new SpriteManager(renderer->get_sdl_renderer());
    sprite_manager->load("body", "client/assets/body.png");
    sprite_manager->load("head", "client/assets/head.png");
    sprite_manager->load("grass", "client/assets/grass.png");
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

    // TODO(chiaradelaurentis): esto va en el TOML
    // [window]
    // width = 800
    // height = 600

    // [map]
    // tile_width = 32
    // tile_height = 32

    // [sprites.body]
    // path = "assets/body.png"
    // frame_width = 27
    // frame_height = 48

    // [sprites.head]
    // path = "assets/head.png"
    // frame_width = 16
    // frame_height = 16

    const int frame_w = 27;
    const int frame_h = 48;
    const int tile_w = 32;
    const int tile_h = 32;
    const int head_frame_w = 27;
    const int head_frame_h = 64;

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
                        my_player_id = static_cast<LoginOkUpdate&>(*u).player_id;
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


    int direction = 0;
    int current_frame = 0;
    int total_frames = 6;

    // cambia fram ede animacion cada 100 ms
    Uint32 last_frame_time = SDL_GetTicks();
    const Uint32 frame_delay = 100;

    // manda como max al servidor un movimiento cada 200 ms
    Uint32 last_move_time = 0;
    const Uint32 move_interval = 200;

    while (running) {
        while (SDL_PollEvent(&event)) {
            // lee eventos SLD: cerrar ventana, tocar una tecla especial, etc.
            // y lee las entradas del jugador (estado del teclado)
            running = input_handler.handle(event);
        }

        const Uint8* keys = SDL_GetKeyboardState(NULL);
        bool moving = false;
        Uint32 now = SDL_GetTicks();

        // Que frecha presiona
        if (keys[SDL_SCANCODE_DOWN]) {

            direction = 0;
            total_frames = 6;
            moving = true;
            if (now - last_move_time >= move_interval) {
                client.do_move(Direction::DOWN);
                last_move_time = now;
            }
        } else if (keys[SDL_SCANCODE_UP]) {
            direction = 1;
            total_frames = 6;
            moving = true;
            if (now - last_move_time >= move_interval) {
                client.do_move(Direction::UP);  // mensaje al servidor
                last_move_time = now;
            }
        } else if (keys[SDL_SCANCODE_LEFT]) {
            direction = 2;
            total_frames = 5;
            moving = true;
            if (now - last_move_time >= move_interval) {
                client.do_move(Direction::LEFT);
                last_move_time = now;
            }
        } else if (keys[SDL_SCANCODE_RIGHT]) {
            direction = 3;
            total_frames = 5;
            moving = true;
            if (now - last_move_time >= move_interval) {
                client.do_move(Direction::RIGHT);
                last_move_time = now;
            }
        }

        // agarra la cola de updates recibidos
        auto& update_queue = client.get_received_updates();
        std::unique_ptr<GameUpdate> update;
        while (update_queue.try_pop(update)) {
            // TODO(chiaradelaurentis): solo contempla updates de movimiento
            if (update->get_type() == UpdateType::MOVED) {
                auto& moved = static_cast<MovedUpdate&>(*update);
                if (moved.get_player_id() == my_player_id) {
                    player_x = moved.get_pos().x * tile_w;
                    player_y = moved.get_pos().y * tile_h;
                }
            }
        }

        // actualiza animacion
        if (moving) {
            if (now - last_frame_time > frame_delay) {
                current_frame = (current_frame + 1) % total_frames;
                last_frame_time = now;
            }
        } else {
            current_frame = 0;
        }

        // ve que pedazo del sprite usar
        int frame_x = current_frame * frame_w;
        int frame_y = direction * frame_h;
        int head_frame_y = direction * head_frame_h;

        // se centra la camara
        camera.center_on(player_x, player_y);
        // convierte coordenadas del mundo a coordenadas de pantalla
        int screen_x = camera.get_screen_x(player_x);
        int screen_y = camera.get_screen_y(player_y);

        renderer->clear();

        int cam_world_x = player_x - width / 2;
        int cam_world_y = player_y - height / 2;
        int start_col = cam_world_x / tile_w - 1;
        int end_col = start_col + width / tile_w + 3;
        int start_row = cam_world_y / tile_h - 1;
        int end_row = start_row + height / tile_h + 3;
        // llenamos la pantalla de pasto
        for (int row = start_row; row <= end_row; row++) {
            for (int col = start_col; col <= end_col; col++) {
                int tx = camera.get_screen_x(col * tile_w);
                int ty = camera.get_screen_y(row * tile_h);
                // TODO(chiaradelaurentis): despues deberia venir el mapa completo
                renderer->draw_frame(sprite_manager->get("grass"), 0, 0, tile_w, tile_h, tx, ty);
            }
        }

        // Body TODO: depende del personaje
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

        hud->draw(73, 100, 30, 80, 3, 450, 1000, 750, width, height);
        renderer->present();
    }
}
