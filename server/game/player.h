#ifndef PLAYER_H
#define PLAYER_H

// Player: representa a un jugador en el juego, con atributos específicos como nombre,
// raza, clase, oro, experiencia, etc. Hereda de Character

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "server/game/character.h"
#include "server/game/inventory.h"
#include "server/game/player_class.h"
#include "server/game/player_race.h"

class Item;

class Player: public Character {
 private:
    std::string name;

    PlayerRace p_race;
    PlayerClass p_class;

    uint64_t gold;
    uint64_t experience;

    bool meditating;

    std::unique_ptr<Inventory> inventory;

 public:
    Player(uint32_t id, const std::string& name, PlayerRace p_race, PlayerClass p_class, int level,
           int max_hp, int max_mana, int strength, int agility, int intelligence, int constitution,
           const Position& position);

    ~Player() override = default;

    bool can_cast_magic() const override;

    int get_defense() const override;

    bool validate_attack_from(int attacker_level) const override;

    Loot drop_loot() override;

    void move(const Position& new_position);

    void add_gold(uint64_t amount);

    bool remove_gold(uint64_t amount);

    void remove_experience(uint64_t amount);

    void add_experience(uint64_t amount);

    void level_up();

    void start_meditating();

    void stop_meditating();

    bool is_meditating() const;

    const std::string& get_name() const;

    PlayerRace get_race() const;

    PlayerClass get_class() const;

    uint64_t get_gold() const;

    uint64_t get_experience() const;

    Inventory& get_inventory();

    const Inventory& get_inventory() const;

    std::vector<std::pair<EquipmentSlot, Item*>> get_equipment() const;

    void equip(Item& item);

    void set_initial_stats(int hp, int mana);

    void restore_mana(int amount);

    void consume_mana(int amount);
};

#endif
