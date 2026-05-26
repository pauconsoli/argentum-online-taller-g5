#include "server/game/inventory.h"

#include <algorithm>
#include <utility>

#include "server/game/items/item.h"
#include "server/game/player.h"

Inventory::Inventory() = default;

int Inventory::find_item(const Item& item) const {
    for (int i = 0; i < static_cast<int>(slots.size()); i++) {
        if (slots[i].item.get() == &item)
            return i;
    }
    return -1;
}

bool Inventory::add_item(std::unique_ptr<Item> item) {
    if (is_full())
        return false;
    slots.push_back(InventorySlot{std::move(item), std::nullopt});
    return true;
}

bool Inventory::equip(Item& item, Player& player) {
    int index = find_item(item);
    if (index == -1)
        return false;  // no está en el inventario

    auto slot = item.get_slot();
    if (!slot.has_value()) {  // si no tiene slot asignado es porque es consumible
        return use_consumable(index, player);
    }

    // Si hay otro item equipado en este slot, desmarcar
    auto it = std::find_if(slots.begin(), slots.end(),
                           [slot](const auto& s) { return s.equipped_slot == slot; });
    if (it != slots.end()) {
        it->equipped_slot = std::nullopt;
    }

    // Marcar este item como equipado
    slots[index].equipped_slot = *slot;

    return true;
}

bool Inventory::unequip(EquipmentSlot slot) {
    auto it = std::find_if(slots.begin(), slots.end(),
                           [slot](const auto& s) { return s.equipped_slot == slot; });
    if (it != slots.end()) {
        it->equipped_slot = std::nullopt;
        return true;
    }
    return false;  // slot no tenía item equipado
}

bool Inventory::use_consumable(int item_index, Player& player) {
    (*slots[item_index].item).use(player);
    slots.erase(slots.begin() + item_index);
    return true;
}

std::unique_ptr<Item> Inventory::remove_item(Item& item) {
    int index = find_item(item);
    if (index == -1)
        return nullptr;

    auto owned = std::move(slots[index].item);
    slots.erase(slots.begin() + index);
    return owned;
}

bool Inventory::is_full() const {
    return slots.size() >= MAX_ITEMS;
}

size_t Inventory::get_size() const {
    return slots.size();
}

const std::vector<InventorySlot>& Inventory::get_slots() const {
    return slots;
}

Item* Inventory::get_equipped_item(EquipmentSlot slot) const {
    auto it = std::find_if(slots.begin(), slots.end(),
                           [slot](const auto& s) { return s.equipped_slot == slot; });
    if (it != slots.end()) {
        return it->item.get();
    }
    return nullptr;
}

bool Inventory::slot_has_item(EquipmentSlot slot) const {
    return get_equipped_item(slot) != nullptr;
}
