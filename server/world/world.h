#ifndef WORLD_H
#define WORLD_H

#include <cstdint>
#include <map>
#include <memory>

#include "common/position.h"
#include "common/direction.h"
#include "common/updates/game_update.h"
#include "server/game/player.h"
#include "world_map.h"


class World {
private:
    WorldMap map;

    std::map<uint32_t, std::unique_ptr<Player>> players;  // player_id -> Player
    std::map<Position, Character*> positions; // position -> Character , para chequear colisiones 

    Position calculate_destination(const Position& current, Direction direction) const;

    bool is_position_occupied(const Position& position) const;

public:
    World(int width, int height);

    void add_player(std::unique_ptr<Player> player);
    void remove_player(uint32_t player_id);
    Player* get_player(uint32_t player_id);
    bool player_exists(uint32_t player_id) const;

    std::unique_ptr<GameUpdate> move_player(uint32_t player_id, Direction direction);

};

#endif