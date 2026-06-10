#include "server/game/items/consumable_item.h"

#include <string>

#include "server/game/player.h"

ConsumableItem::ConsumableItem(const std::string& name, ConsumableType type, int restore):
    Item(name), type(type), restore(restore) {}

ConsumableItem::~ConsumableItem() = default;

ConsumableType ConsumableItem::get_type() const {
    return type;
}

int ConsumableItem::get_restore() const {
    return restore;
}

std::optional<EquipmentSlot> ConsumableItem::get_slot() const {
    return std::nullopt;  // no se equipa, se consume
}

void ConsumableItem::use(Player& player) {
    if (type == ConsumableType::HEALTH) {
        player.heal(restore);
    } else {
        player.restore_mana(restore);
    }
}
