#include "sdl_config.h"

#include <toml++/toml.hpp>

SdlConfig SdlConfig::load(const std::string& path) {
    SdlConfig cfg;
    try {
        auto root = toml::parse_file(path);

        if (auto width = root["sdl"]["window"]["width"].value<int>())
            cfg.window_width = *width;
        if (auto height = root["sdl"]["window"]["height"].value<int>())
            cfg.window_height = *height;
        if (auto fullscreen = root["sdl"]["window"]["fullscreen"].value<bool>())
            cfg.fullscreen = *fullscreen;

        if (auto tileWidth = root["sdl"]["render"]["tile_width"].value<int>())
            cfg.tile_width = *tileWidth;
        if (auto tileHeight = root["sdl"]["render"]["tile_height"].value<int>())
            cfg.tile_height = *tileHeight;
        if (auto targetFps = root["sdl"]["render"]["target_fps"].value<int>())
            cfg.target_fps = static_cast<Uint32>(*targetFps);

        if (auto frameDelay = root["sdl"]["animation"]["frame_delay_ms"].value<int>())
            cfg.frame_delay_ms = static_cast<Uint32>(*frameDelay);

        if (auto moveInterval = root["sdl"]["input"]["move_interval_ms"].value<int>())
            cfg.move_interval_ms = static_cast<Uint32>(*moveInterval);

    } catch (...) {
        // Archivo ausente o malformado: se usan los valores por defecto del struct.
    }
    return cfg;
}
