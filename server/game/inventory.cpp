#include "server/game/inventory.h"

#include <algorithm>
#include <utility>

#include "server/game/game_config.h"
#include "server/game/items/item.h"
#include "server/game/player.h"

Inventory::Inventory() {
    int max_items = GameConfig::get_instance().get_max_inventory_items();
    for (int i = 0; i < max_items; ++i) {
        slots.push_back(InventorySlot{nullptr, 0, std::nullopt});
    }
}

int Inventory::find_item_by_ref(const Item& item) const {
    for (int i = 0; i < static_cast<int>(slots.size()); i++) {
        if (slots[i].item.get() ==
            &item)  // lo encuentra con la referencia puntual del item, para equipar/desquipar
            return i;
    }
    return -1;
}

int Inventory::find_item_by_type(const Item& item) const {
    for (int i = 0; i < static_cast<int>(slots.size()); i++) {
        if (slots[i].item &&
            slots[i].item->get_name() ==
                item.get_name()) {  // lo encuentra por tipo/nombre, para stackearlo
            return i;
        }
    }
    return -1;
}

bool Inventory::add_item(std::unique_ptr<Item> item, int quantity) {
    if (item == nullptr || quantity <= 0)
        return false;

    if (!can_add_item(*item))
        return false;

    // solo stackeo items no equipables, como pociones
    if (!item->get_slot().has_value()) {
        int index = find_item_by_type(*item);

        if (index != -1) {
            slots[index].quantity += quantity;
            return true;
        }
    }

    for (int i = 0; i < static_cast<int>(slots.size()); i++) {
        if (!slots[i].item) {
            slots[i].item = std::move(item);
            slots[i].quantity = quantity;
            slots[i].equipped_slot = std::nullopt;
            return true;
        }
    }

    return true;
}

bool Inventory::can_add_item(const Item& item) const {
    if (!item.get_slot().has_value()) {
        if (find_item_by_type(item) != -1) {
            return true;
        }
    }
    return !is_full();
}

bool Inventory::equip(Item& item, Player& player) {
    int index = find_item_by_ref(item);
    if (index == -1)
        return false;  // no está en el inventario

    auto slot = item.get_slot();
    if (!slot.has_value()) {  // si no tiene slot asignado es porque es consumible
        return use_consumable(index, player);
    }

    // acá veo si hay otro item equipado en ese slot, para desmarcarlo (swap), distinto al item que
    // quiero equipar
    auto it = std::find_if(slots.begin(), slots.end(), [slot, &item](const auto& s) {
        return s.item && s.equipped_slot == slot && s.item.get() != &item;
    });
    if (it != slots.end()) {
        it->equipped_slot = std::nullopt;
    }

    slots[index].equipped_slot = *slot;  // EQUIPADO

    return true;
}

bool Inventory::unequip(EquipmentSlot slot) {
    auto it = std::find_if(slots.begin(), slots.end(),
                           [slot](const auto& s) { return s.item && s.equipped_slot == slot; });
    if (it != slots.end()) {
        it->equipped_slot = std::nullopt;
        return true;
    }
    return false;  // slot no tenía item equipado
}

bool Inventory::use_consumable(int item_index, Player& player) {
    (*slots[item_index].item).use(player);
    remove_item(*slots[item_index].item);
    return true;
}

std::unique_ptr<Item> Inventory::remove_item(
    Item& item) {  // SACSAR UNO SOLO por ej si voy usando pociones
    int index = find_item_by_ref(item);
    if (index == -1)
        return nullptr;

    if (slots[index].quantity > 1) {
        slots[index].quantity--;
        return nullptr;  // sigue en el inventario
    }

    auto owned = std::move(
        slots[index].item);  // si entra acá era la última unidad, se saca el item del inventario
    slots[index].quantity = 0;
    slots[index].equipped_slot = std::nullopt;
    return owned;
}

bool Inventory::is_full() const {
    return std::all_of(slots.begin(), slots.end(),
                       [](const auto& slot) { return slot.item != nullptr; });
}

size_t Inventory::get_size() const {
    return static_cast<size_t>(std::count_if(
        slots.begin(), slots.end(), [](const auto& slot) { return slot.item != nullptr; }));
}

const std::vector<InventorySlot>& Inventory::get_slots() const {
    return slots;
}

Item* Inventory::get_equipped_item(EquipmentSlot slot) const {
    auto it = std::find_if(slots.begin(), slots.end(),
                           [slot](const auto& s) { return s.item && s.equipped_slot == slot; });
    if (it != slots.end()) {
        return it->item.get();
    }
    return nullptr;
}

std::vector<std::pair<EquipmentSlot, Item*>> Inventory::get_equipped_items() const {
    std::vector<std::pair<EquipmentSlot, Item*>> result;
    for (const auto& slot : slots) {
        if (slot.item && slot.equipped_slot.has_value()) {
            result.push_back({slot.equipped_slot.value(), slot.item.get()});
        }
    }
    return result;
}

bool Inventory::slot_has_item(EquipmentSlot slot) const {
    return get_equipped_item(slot) != nullptr;
}

InventorySlot Inventory::pop_slot(int slot_index) {
    if (slot_index < 0 || slot_index >= static_cast<int>(slots.size())) {
        return InventorySlot{nullptr, 0, std::nullopt};  // índice inválido
    }

    InventorySlot slot_copy{std::move(slots[slot_index].item), slots[slot_index].quantity,
                            slots[slot_index].equipped_slot};
    slots[slot_index].quantity = 0;
    slots[slot_index].equipped_slot = std::nullopt;
    return slot_copy;
}

std::vector<InventorySlot> Inventory::drop_all() {
    std::vector<InventorySlot> dropped;
    for (auto& slot : slots) {
        if (slot.item) {
            dropped.push_back(
                InventorySlot{std::move(slot.item), slot.quantity, slot.equipped_slot});
            slot.quantity = 0;
            slot.equipped_slot = std::nullopt;
        }
    }
    return dropped;
}
