#ifndef WORLD_H
#define WORLD_H

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <vector>

#include "common/attack_result.h"
#include "common/direction.h"
#include "common/position.h"
#include "common/updates/game_update.h"
#include "server/game/items/item.h"
#include "server/game/loot.h"
#include "server/game/player.h"
#include "world_map.h"

struct GroundItem {  // por ahora así, no se si es lo mejor
    uint64_t gold = 0;
    std::unique_ptr<Item> item = nullptr;
    int quantity = 0;
};

class World {
 private:
    WorldMap map;

    std::map<uint32_t, std::unique_ptr<Player>> players;  // player_id -> Player
    std::vector<std::vector<bool>>
        occupied;  // matriz booleana para saber si una posición está ocupada (true) o no (false)

    std::map<Position, GroundItem> ground_items;

    Position calculate_destination(const Position& current, Direction direction) const;

    bool is_position_occupied(const Position& position) const;

    bool is_in_range_for_attack(const Player* attacker, const Character* target) const;

    std::optional<Position> find_closest_free_ground(const Position& start) const;

    AttackStatus validate_attack_conditions(const Player* attacker, const Character* target,
                                            bool is_healing) const;
    AttackStatus consume_weapon_mana(Player* attacker);
    void handle_target_death(Player* attacker, Character* target);
    int handle_successful_attack(Player* attacker, Character* target, int damage);

 public:
    World(int width, int height);
    explicit World(WorldMap map);  // para cargar un mapa ya creado

    const WorldMap& get_map() const {
        return map;
    }

    void add_player(std::unique_ptr<Player> player);
    void remove_player(uint32_t player_id);
    Player* get_player(uint32_t player_id);
    std::vector<Player*> get_players();
    bool player_exists(uint32_t player_id) const;

    Character* get_character(uint32_t id);

    Position get_spawn_position() const;

    const std::map<Position, GroundItem>& get_ground_items() const;

    void drop_loot_in_world(const Position& center, Loot loot);

    void add_ground_item(const Position& pos, GroundItem item);

    bool pick_up_item(uint32_t player_id);
    bool drop_item(uint32_t player_id, int slot_index);
    bool equip_item(uint32_t player_id, int slot_index);

    bool move_player(uint32_t player_id, Direction direction);

    AttackResult attack(uint32_t attacker_id, uint32_t target_id);

    void update(float tick_seconds);

    // solo lo uso para poner celdas bloqueantes en el mapa en los tests
    void set_cell(const Position& pos, const Cell& cell);
};

#endif
