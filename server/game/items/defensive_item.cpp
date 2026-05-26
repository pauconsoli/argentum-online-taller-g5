#include "server/game/items/defensive_item.h"

#include "server/game/player.h"

DefensiveItem::DefensiveItem(const std::string& name, int min_defense, int max_defense,
                             EquipmentSlot slot):
    Item(name), min_defense(min_defense), max_defense(max_defense), slot(slot) {}

DefensiveItem::~DefensiveItem() = default;

int DefensiveItem::get_min_defense() const {
    return min_defense;
}

int DefensiveItem::get_max_defense() const {
    return max_defense;
}

std::optional<EquipmentSlot> DefensiveItem::get_slot() const {
    return slot;
}

void DefensiveItem::use(Player& player) {
    player.equip(*this);
}
