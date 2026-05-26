#include "server/game/inventory.h"
#include "server/game/equipment.h"
#include "server/game/items/item.h"
#include "server/game/player.h"

Inventory::Inventory(Equipment& eq) : equipment(eq) {}

int Inventory::find_item(const Item& item) const {
    for (int i = 0; i < static_cast<int>(items.size()); i++) {
        if (items[i].get() == &item) return i;
    }
    return -1;
}

bool Inventory::add_item(std::unique_ptr<Item> item) {
    if (is_full()) return false;
    items.push_back(std::move(item));
    return true;
}

bool Inventory::equip(Item& item, Player& player) {   // equipar puede ser que se agregue al equipment o que se consuma dependiendo del tipo de item
    int index = find_item(item);
    if (index == -1) return false; // no está en el inventario

    auto slot = item.get_slot();
    if (!slot.has_value()) {  // si no tiene slot asignado es porque es consumible
        return use_consumable(index, player);
    }

    Item* prev = equipment.equip(items[index].get(), *slot);  // tomo el item anterior del slot si existe

    auto equipped_item = std::move(items[index]);
    items.erase(items.begin() + index);
    equipped_item.release();  

    if (prev != nullptr) {
        if (!is_full()) {
            items.push_back(std::unique_ptr<Item>(prev));
        } else {
            delete prev;  // sería el drop
        }
    }

    return true;
}

bool Inventory::unequip(EquipmentSlot slot) {
    Item* item = equipment.unequip(slot);
    if (item == nullptr) return false;  // slot vacío

    if (!is_full()) {
        items.push_back(std::unique_ptr<Item>(item)); // si el inventario no está lleno vuelve al inventario
    } else {
        delete item;  //drop
    }
    return true;
}

bool Inventory::use_consumable(int item_index, Player& player) {
    (*items[item_index]).use(player);    
    items.erase(items.begin() + item_index);
    return true;
}

std::unique_ptr<Item> Inventory::remove_item(Item& item) {
    int index = find_item(item);
    if (index == -1) return nullptr;
    auto owned = std::move(items[index]);
    items.erase(items.begin() + index);
    return owned;
}

bool Inventory::is_full() const {
    return items.size() >= MAX_ITEMS;
}

size_t Inventory::get_size() const { return items.size(); }

const std::vector<std::unique_ptr<Item>>& Inventory::get_items() const {
    return items;
}