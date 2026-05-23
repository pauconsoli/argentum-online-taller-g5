#include "server/game/inventory.h"
#include "server/game/player.h"

Inventory::Inventory() = default;

int Inventory::find_item(Item& item) {
    for (size_t i = 0; i < items.size(); ++i) {
        if (items[i].get() == &item) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool Inventory::add_item(std::unique_ptr<Item> item) {
    if (is_full()) {
        return false;
    }
    
    items.push_back(std::move(item));
    return true;
}

bool Inventory::equip(Item& item) {
    if (!contains_item(item)) {
        return false;
    }

    auto slot = item.get_slot();
    if (!slot.has_value()) {
        return false;  // No es equipable (es consumible)
    }

    if (equipment.has(slot.value())) {
        unequip(slot.value());
    }

    equipment.set(slot.value(), &item);
    return true;
}

bool Inventory::unequip(EquipmentSlot slot) {
    if (!equipment.has(slot)) {
        return false;
    }
    
    equipment.unset(slot);
    return true;
}

bool Inventory::use_consumable(Item& item, Player& player) {
    if (!contains_item(item)) {
        return false;
    }
    
    auto slot = item.get_slot();
    if (slot.has_value()) {
        return false;  // No es consumible (es equipable)
    }

    item.use(player);
    
    remove_item(item);
    
    return true;
}

bool Inventory::remove_item(Item& item) {

    equipment.remove(&item);

    int index = find_item(item);
    if (index == -1) {
        return false;
    }
    
    items.erase(items.begin() + index);
    return true;
}

Item* Inventory::get_equipped(EquipmentSlot slot) const {
    return equipment.get(slot);
}

const std::vector<std::unique_ptr<Item>>& Inventory::get_items() const {
    return items;
}

size_t Inventory::get_size() const {
    return items.size();
}

bool Inventory::is_full() const {
    return items.size() >= MAX_ITEMS;
}

bool Inventory::contains_item(Item& item) const {
    for (const auto& inv_item : items) {
        if (inv_item.get() == &item) {
            return true;
        }
    }
    return false;
}
