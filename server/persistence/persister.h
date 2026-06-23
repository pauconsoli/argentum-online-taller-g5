#ifndef PERSISTER_H
#define PERSISTER_H

#include <optional>
#include <string>

#include "server/persistence/player_save.h"

class Persister {
 public:
    explicit Persister(std::string save_path);

    std::optional<WorldSnapshot> try_load();

    void save(const WorldSnapshot& snapshot);
    std::optional<PlayerSave> find_player_save(const std::string& nick,
                                                const std::string& match_name);

 private:
    std::string save_path;
    std::optional<WorldSnapshot> cached;
    bool cache_loaded = false;
};

#endif
