#ifndef WORLD_SERIALIZER_H
#define WORLD_SERIALIZER_H

#include <nlohmann/json.hpp>

#include "server/persistence/player_save.h"

namespace WorldSerializer {

nlohmann::json to_json(const WorldSnapshot& s);
WorldSnapshot from_json(const nlohmann::json& j);

nlohmann::json to_json(const PlayerSave& p);
PlayerSave player_from_json(const nlohmann::json& j);

} 

#endif
