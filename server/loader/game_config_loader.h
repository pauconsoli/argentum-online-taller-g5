#ifndef GAME_CONFIG_LOADER_H
#define GAME_CONFIG_LOADER_H

#include <string>

class GameConfigLoader {
 public:
    static void load(const std::string& config_path);
};

#endif
