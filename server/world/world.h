#ifndef WORLD_H
#define WORLD_H

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "common/attack_result.h"
#include "common/direction.h"
#include "common/position.h"
#include "common/updates/game_update.h"
#include "server/game/bank.h"
#include "server/game/items/item.h"
#include "server/game/loot.h"
#include "server/game/npcs/city_npc.h"
#include "server/game/npcs/npc.h"
#include "server/game/player.h"
#include "world_map.h"

struct GroundItem {  // por ahora así, no se si es lo mejor
    uint64_t gold = 0;
    std::unique_ptr<Item> item = nullptr;
    int quantity = 0;
};

struct PendingResurrection {
    float timer;
    Position destination;
};

class World {
 private:
    WorldMap map;

    std::map<uint32_t, std::unique_ptr<Player>> players;  // player_id -> Player
    std::vector<std::vector<bool>>
        occupied;  // matriz booleana para saber si una posición está ocupada (true) o no (false)

    std::map<uint32_t, std::unique_ptr<NPC>> npcs;  // npc_id -> NPC
    Bank bank;

    float npc_spawn_timer = 0.0f;
    uint32_t next_npc_id = 1000;

    std::map<Position, GroundItem> ground_items;

    std::map<uint32_t, PendingResurrection> pending_resurrections;

    Position calculate_destination(const Position& current, Direction direction) const;

    bool is_position_occupied(const Position& position) const;

    bool is_in_range_for_attack(const Character* attacker, const Character* target) const;

    std::optional<Position> find_closest_free_ground(const Position& start) const;
    std::optional<Position> find_closest_unoccupied_position(const Position& start) const;

    // funcion para no repetir en las dos anteriores, que la reutilizan
    std::optional<Position> find_closest_free_position(
        const Position& start, const std::function<bool(const Position&)>& is_free_condition) const;

    AttackStatus validate_attack_conditions(const Character* attacker,
                                            const Character* target) const;
    void handle_target_death(Character* attacker, Character* target);
    int handle_successful_attack(Character* attacker, Character* target, int damage);

    void update_players(float tick_seconds);
    void update_resurrections(float tick_seconds);
    std::vector<AttackResult> update_npcs(float tick_seconds);
    void update_spawning(float tick_seconds);

    void npc_move_towards(NPC* npc, const Position& target_pos);
    AttackResult npc_attack(NPC* npc, Character* target);
    void try_spawn_npc();
    std::optional<Position> find_random_spawn_position(
        const std::vector<std::string>& allowed_zones) const;

    void spawn_city_npcs();
    void spawn_initial_hostile_npcs();

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

    bool move_character(uint32_t character_id, Direction direction);

    bool teleport_player(uint32_t player_id, const Position& dest);
    bool start_resurrection(uint32_t player_id);

    AttackResult attack(uint32_t attacker_id, uint32_t target_id);
    AttackResult heal(uint32_t healer_id, uint32_t target_id);

    void add_npc(std::unique_ptr<NPC> npc);
    NPC* get_npc(uint32_t npc_id);

    std::vector<NPC*> get_npcs();

    CityNPC* get_city_npc(uint32_t npc_id);
    std::vector<Player*> get_players_near(const Position& pos, float range) const;
    InteractResult interact_with_npc(uint32_t player_id, uint32_t npc_id, NPCInteraction type,
                                     const std::string& arg, int amount);
    Bank& get_bank();

    std::vector<AttackResult> update(float tick_seconds);

    // solo lo uso para poner celdas bloqueantes en el mapa en los tests
    void set_cell(const Position& pos, const Cell& cell);
};

#endif
