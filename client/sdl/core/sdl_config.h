#ifndef SDL_CONFIG_H
#define SDL_CONFIG_H

#include <string>

#include <SDL2/SDL.h>

struct SdlConfig {
    int window_width = 800;
    int window_height = 600;
    bool fullscreen = false;

    int tile_width = 32;
    int tile_height = 32;

    Uint32 target_fps = 30;
    Uint32 frame_delay_ms = 100;
    Uint32 move_interval_ms = 100;

    Uint32 frame_time_ms() const {
        return 1000 / target_fps;
    }

    // Carga desde TOML; si el archivo falta o es inválido, devuelve defaults seguros.
    static SdlConfig load(const std::string& path);
};

#endif
