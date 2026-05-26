#include "server/game/items/weapon.h"
#include "server/game/player.h"

Weapon::Weapon(const std::string& name):
        Item(name) {}

Weapon::~Weapon() = default;

std::optional<EquipmentSlot> Weapon::get_slot() const {
    return EquipmentSlot::WEAPON;
}

void Weapon::use(Player& player) {
    player.equip(*this);
}