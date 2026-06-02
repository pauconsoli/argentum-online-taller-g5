#ifndef INVENTORY_H
#define INVENTORY_H
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "server/game/items/equipment_slot.h"
#include "server/game/items/item.h"

class Player;

// InventorySlot: item con cantidad y flag de si está equipado
struct InventorySlot {
    std::unique_ptr<Item> item;                  // template/referencia del item
    int quantity = 1;                            // cantidad de este item (stackable)
    std::optional<EquipmentSlot> equipped_slot;  // nullopt si no está equipado,
                                                 // tiene el slot si está equipado (solo 1 equipado)
};

class Inventory {
 private:
    std::vector<InventorySlot> slots;

    int find_item_by_ref(const Item& item) const;

    int find_item_by_type(const Item& item) const;

    bool use_consumable(int item_index, Player& player);

 public:
    Inventory();
    ~Inventory() = default;

    Inventory(const Inventory&) = delete;
    Inventory& operator=(const Inventory&) = delete;

    bool add_item(std::unique_ptr<Item> item,
                  int quantity = 1);  // por defecto es 1, pero se puede agregar más de una unidad
                                      // para el caso del loot

    bool equip(Item& item, Player& player);

    bool unequip(EquipmentSlot slot);

    std::unique_ptr<Item> remove_item(Item& item);

    bool is_full() const;
    size_t get_size() const;
    const std::vector<InventorySlot>& get_slots() const;

    Item* get_equipped_item(EquipmentSlot slot) const;

    std::vector<std::pair<EquipmentSlot, Item*>> get_equipped_items() const;

    bool slot_has_item(EquipmentSlot slot) const;

    InventorySlot pop_slot(int slot_index);
};

#endif
