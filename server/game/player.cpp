#include "player.h"

#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

#include "server/game/game_formulas.h"
#include "server/game/inventory.h"

Player::Player(uint32_t id, const std::string& name, PlayerRace race, PlayerClass player_class,
               int level, int max_hp, int max_mana, int strength, int agility, int intelligence,
               int constitution, const Position& position):
    Character(id, level, max_hp, max_mana, strength, agility, intelligence, constitution, position),
    name(name),
    p_race(race),
    p_class(player_class),
    gold(0),
    experience(0),
    meditating(false),
    inventory(std::make_unique<Inventory>()) {}

bool Player::can_cast_magic() const {
    return p_class != PlayerClass::WARRIOR;
}  // si no es guerrero, puede usar magia

int Player::get_defense() const {
    return GameFormulas::calculate_defense(*this);
}

bool Player::validate_attack_from(int attacker_level) const {
    return GameFormulas::can_attack_by_level(attacker_level, this->get_level());
}

Loot Player::drop_loot() {
    Loot loot;

    loot.dropped_gold = GameFormulas::calculate_player_dropped_gold(*this);
    if (loot.dropped_gold > 0) {
        remove_gold(loot.dropped_gold);  // descuenta el oro del jugador que cae al suelo al morir
    }

    uint64_t dropped_exp = GameFormulas::calculate_player_dropped_experience(*this);
    if (dropped_exp > 0) {
        remove_experience(dropped_exp);  // descuenta la exp que pierde el jugador al morir
    }

    loot.dropped_items = inventory->drop_all();

    return loot;
}

void Player::move(const Position& new_position) {
    position = new_position;
}

void Player::add_gold(uint64_t amount) {
    gold += amount;
}

bool Player::remove_gold(uint64_t amount) {
    if (amount > gold) {
        return false;
    }

    gold -= amount;
    return true;
}

// no se puede perder más experiencia que la que se tiene, ni menos que el piso del nivel (o sea
// tope del nivel anterior)
void Player::remove_experience(uint64_t amount) {
    uint64_t level_floor =
        (get_level() > 1) ? GameFormulas::calculate_level_up_limit(get_level() - 1) : 0;

    if (experience > amount && (experience - amount) >= level_floor) {
        experience -= amount;
    } else {
        experience = level_floor;
    }
}

void Player::add_experience(uint64_t amount) {
    experience += amount;

    while (experience >= GameFormulas::calculate_level_up_limit(get_level())) {
        level_up();
    }
}

void Player::level_up() {

    int new_level = get_level() + 1;
    set_level(new_level);

    int new_max_hp = GameFormulas::calculate_max_hp(*this);
    int new_max_mana = GameFormulas::calculate_max_mana(*this);
    // TODO(Pau): falta el ORO

    set_initial_stats(new_max_hp, new_max_mana);
    // al subir de nivel, el jugador recupera toda su vida y mana (sería el nuevo max)
    // ver si esto queda así o si tengo que cambiar el set_initial_stats o usar otros setters
}

void Player::start_meditating() {
    if (can_cast_magic()) {
        meditating = true;
    }
}

void Player::stop_meditating() {
    meditating = false;
}

bool Player::is_meditating() const {
    return meditating;
}

const std::string& Player::get_name() const {
    return name;
}

PlayerRace Player::get_race() const {
    return p_race;
}

PlayerClass Player::get_class() const {
    return p_class;
}

uint64_t Player::get_gold() const {
    return gold;
}

uint64_t Player::get_experience() const {
    return experience;
}

// SOBRECARGA: una por si quiero modificar el inventario, otra solo para lectura
// (ej la uso en las fórmulas)
Inventory& Player::get_inventory() {
    return *inventory;
}

const Inventory& Player::get_inventory() const {
    return *inventory;
}

void Player::equip(Item& item) {
    inventory->equip(item, *this);
}

std::vector<std::pair<EquipmentSlot, Item*>> Player::get_equipment() const {
    return inventory->get_equipped_items();
}

void Player::set_initial_stats(int hp, int mana) {
    this->max_hp = hp;
    this->current_hp = hp;
    this->max_mana = mana;
    this->current_mana = mana;
}

void Player::restore_mana(int amount) {
    if (amount <= 0 || is_dead() || !can_cast_magic()) {
        return;
    }
    current_mana += amount;
    if (current_mana > max_mana) {
        current_mana = max_mana;
    }
}

void Player::consume_mana(int amount) {
    if (amount <= 0 || is_dead() || !can_cast_magic()) {
        return;
    }
    current_mana -= amount;
    if (current_mana < 0) {
        current_mana = 0;
    }
}
