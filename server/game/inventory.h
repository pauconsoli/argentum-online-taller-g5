#ifndef INVENTORY_H
#define INVENTORY_H
#include <memory>
#include <optional>
#include <vector>

#include "server/game/items/equipment_slot.h"
#include "server/game/items/item.h"

#define MAX_INVENTORY_ITEMS 20

class Player;

// InventorySlot: item con flag de si está equipado
struct InventorySlot {
    std::unique_ptr<Item> item;
    std::optional<EquipmentSlot> equipped_slot;  // nullopt si no está equipado,
                                                 // tiene el slot si está equipado
};

class Inventory {
 private:
    static constexpr size_t MAX_ITEMS = MAX_INVENTORY_ITEMS;
    std::vector<InventorySlot> slots;

    // USA un consumible (desp lo elimina del inventario)
    bool use_consumable(int item_index, Player& player);
    int find_item(const Item& item) const;

 public:
    Inventory();
    ~Inventory() = default;

    Inventory(const Inventory&) = delete;
    Inventory& operator=(const Inventory&) = delete;

    bool add_item(std::unique_ptr<Item> item);

    // Marca el item como equipado en su slot correspondiente
    // Si había otro item equipado en ese slot, lo desactualiza
    // retorna false si el item no está o viola restricción
    bool equip(Item& item, Player& player);

    // Desmarca el item del slot indicado
    // retorna false si el slot está vacío
    bool unequip(EquipmentSlot slot);

    // Saca el item del inventario (para dropearlo, retorna ownership)
    std::unique_ptr<Item> remove_item(Item& item);

    bool is_full() const;
    size_t get_size() const;
    const std::vector<InventorySlot>& get_slots() const;

    // Obtiene el item equipado en un slot, o nullptr
    Item* get_equipped_item(EquipmentSlot slot) const;

    // Verifica si hay un item equipado en un slot
    bool slot_has_item(EquipmentSlot slot) const;
};

#endif
