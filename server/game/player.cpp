#include "player.h"

#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

#include "server/game/game_formulas.h"
#include "server/game/inventory.h"
#include "server/game/items/weapon.h"

Player::Player(uint32_t id, const std::string& name, PlayerRace race, PlayerClass player_class,
               int level, int max_hp, int max_mana, int strength, int agility, int intelligence,
               int constitution, const Position& position):
    Character(id, level, max_hp, position),
    name(name),
    p_race(race),
    p_class(player_class),
    gold(0),
    experience(0),
    current_mana(max_mana),
    max_mana(max_mana),
    strength(strength),
    agility(agility),
    intelligence(intelligence),
    constitution(constitution),
    meditating(false),
    partial_hp_regen(0.0f),
    partial_mana_regen(0.0f),
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

int Player::get_agility() const {
    return agility;
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

bool Player::is_healing_attack() const {
    Weapon* weapon = get_equipped_weapon();
    return weapon ? weapon->is_healing() : false;
}

bool Player::is_ranged_attack() const {
    Weapon* weapon = get_equipped_weapon();
    return weapon ? weapon->is_ranged() : false;
}

bool Player::is_magic_attack() const {
    Weapon* weapon = get_equipped_weapon();
    return weapon ? weapon->is_magic() : false;
}

std::string Player::get_attack_name() const {
    Weapon* weapon = get_equipped_weapon();
    return weapon ? weapon->get_attack_name() : "";
}

int Player::calculate_base_damage() const {
    Weapon* weapon = get_equipped_weapon();
    if (weapon) {
        return weapon->apply_effect(*this).damage;
    }
    return GameFormulas::calculate_damage(*this);
}

int Player::calculate_base_healing() const {
    Weapon* weapon = get_equipped_weapon();
    return weapon ? weapon->apply_effect(*this).healing : 0;
}

AttackStatus Player::consume_attack_resources() {
    Weapon* weapon = get_equipped_weapon();
    if (weapon && weapon->get_mana_cost() > 0) {
        if (current_mana < weapon->get_mana_cost())
            return AttackStatus::NO_MANA;
        consume_mana(weapon->get_mana_cost());
    }
    return AttackStatus::SUCCESS;
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

void Player::resurrect() {
    if (dead) {
        dead = false;
        current_hp = max_hp;
        current_mana = max_mana;
    }
}

bool Player::is_meditating() const {
    return meditating;
}

void Player::add_partial_hp(float hp) {
    partial_hp_regen += hp;
}

float Player::get_partial_hp() const {
    return partial_hp_regen;
}

void Player::decrease_partial_hp(float hp) {
    partial_hp_regen -= hp;
}

void Player::add_partial_mana(float mana) {
    partial_mana_regen += mana;
}

float Player::get_partial_mana() const {
    return partial_mana_regen;
}

void Player::decrease_partial_mana(float mana) {
    partial_mana_regen -= mana;
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

int Player::get_current_mana() const {
    return current_mana;
}

int Player::get_max_mana() const {
    return max_mana;
}

int Player::get_strength() const {
    return strength;
}

int Player::get_intelligence() const {
    return intelligence;
}

int Player::get_constitution() const {
    return constitution;
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

Weapon* Player::get_equipped_weapon() const {
    Item* item = inventory->get_equipped_item(EquipmentSlot::WEAPON);
    return static_cast<Weapon*>(item);  // yo se que ese item es SI O SI un arma, magica o normal
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
