#include "game_client.h"
#include <stdexcept>

GameClient::GameClient(int width, int height) : 
    window(nullptr), renderer(nullptr), 
    camera(width, height), player_x(400), player_y(300) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error(SDL_GetError());
    }
    window = SDL_CreateWindow(
        "Argentum Online - G5",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_SHOWN
    );
    if (window == nullptr) {
        SDL_Quit();
        throw std::runtime_error(SDL_GetError());
    }
    renderer = new Renderer(window);
    renderer->load_texture("../client/assets/body.png");
}

GameClient::~GameClient() {
    delete renderer;
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void GameClient::run() {
    bool running = true;
    SDL_Event event;

    int frame_w = 31;
    int frame_h = 45;
    int frame_x = 0;
    int frame_y = 0;
    int speed = 3;

    int current_frame = 0;
    int total_frames = 6;
    Uint32 last_frame_time = SDL_GetTicks();
    Uint32 frame_delay = 100; 

    while (running) {
        while (SDL_PollEvent(&event)) {
            running = input_handler.handle(event);
        }

        const Uint8* keys = SDL_GetKeyboardState(NULL);
        bool moving = false;

        if (keys[SDL_SCANCODE_UP]) {
            player_y -= speed;
            frame_y = 45; 
            moving = true;
        } else if (keys[SDL_SCANCODE_DOWN]) {
            player_y += speed;
            frame_y = 0; 
            moving = true;
        } else if (keys[SDL_SCANCODE_LEFT]) {
            player_x -= speed;
            frame_y = 90;
            moving = true;
        } else if (keys[SDL_SCANCODE_RIGHT]) {
            player_x += speed;
            frame_y = 135; 
            moving = true;
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

        frame_x = current_frame * frame_w;

        camera.center_on(player_x, player_y);
        int screen_x = camera.get_screen_x(player_x);
        int screen_y = camera.get_screen_y(player_y);

        renderer->clear();
        renderer->draw_frame(frame_x, frame_y, frame_w, frame_h, screen_x, screen_y);
        renderer->present();
    }
}