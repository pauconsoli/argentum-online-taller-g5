#include "player.h"

#include <utility>
#include <vector>

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

void Player::add_experience(uint64_t amount) {
    experience += amount;
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

Inventory& Player::get_inventory() {
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

void Player::restore_health(int amount) {
    (void) amount;
    // TODO(Pau)
}

void Player::restore_mana(int amount) {
    (void) amount;
    // TODO(Pau)
}
