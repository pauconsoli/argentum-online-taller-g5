#include "sdl_config.h"

#include <toml++/toml.hpp>

SdlConfig SdlConfig::load(const std::string& path) {
    SdlConfig cfg;
    try {
        auto root = toml::parse_file(path);

        if (auto v = root["sdl"]["window"]["width"].value<int>())
            cfg.window_width = *v;
        if (auto v = root["sdl"]["window"]["height"].value<int>())
            cfg.window_height = *v;
        if (auto v = root["sdl"]["window"]["fullscreen"].value<bool>())
            cfg.fullscreen = *v;

        if (auto v = root["sdl"]["render"]["tile_width"].value<int>())
            cfg.tile_width = *v;
        if (auto v = root["sdl"]["render"]["tile_height"].value<int>())
            cfg.tile_height = *v;
        if (auto v = root["sdl"]["render"]["target_fps"].value<int>())
            cfg.target_fps = static_cast<Uint32>(*v);

        if (auto v = root["sdl"]["animation"]["frame_delay_ms"].value<int>())
            cfg.frame_delay_ms = static_cast<Uint32>(*v);

        if (auto v = root["sdl"]["input"]["move_interval_ms"].value<int>())
            cfg.move_interval_ms = static_cast<Uint32>(*v);

    } catch (...) {
        // Archivo ausente o malformado: se usan los valores por defecto del struct.
    }
    return cfg;
}
