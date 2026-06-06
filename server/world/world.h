#ifndef WORLD_H
#define WORLD_H

#include <cstdint>
#include <map>
#include <memory>
#include <vector>

#include "common/attack_result.h"
#include "common/direction.h"
#include "common/position.h"
#include "common/updates/game_update.h"
#include "server/game/player.h"
#include "world_map.h"


class World {
 private:
    WorldMap map;

    std::map<uint32_t, std::unique_ptr<Player>> players;  // player_id -> Player
    std::vector<std::vector<bool>>
        occupied;  // matriz booleana para saber si una posición está ocupada (true) o no (false)

    Position calculate_destination(const Position& current, Direction direction) const;

    bool is_position_occupied(const Position& position) const;

    bool is_in_range_for_attack(const Player* attacker, const Player* target) const;

 public:
    World(int width, int height);
    explicit World(WorldMap map);  // para cargar un mapa ya creado

    void add_player(std::unique_ptr<Player> player);
    void remove_player(uint32_t player_id);
    Player* get_player(uint32_t player_id);
    std::vector<Player*> get_players();
    bool player_exists(uint32_t player_id) const;

    Position get_spawn_position() const;

    bool move_player(uint32_t player_id, Direction direction);

    AttackResult attack_player(uint32_t attacker_id, uint32_t target_id);

    void update(float tick_seconds);

    // solo lo uso para poner celdas bloqueantes en el mapa en los tests
    void set_cell(const Position& pos, const Cell& cell);
};

#endif
