#ifndef INVENTORY_H
#define INVENTORY_H
#include <memory>
#include <vector>

#include "server/game/items/item.h"

#define MAX_INVENTORY_ITEMS 20  // esto despues hay que sacarlo a un config o algo asi

class Player;
class Equipment;

class Inventory {
 private:
    static constexpr size_t MAX_ITEMS = MAX_INVENTORY_ITEMS;
    std::vector<std::unique_ptr<Item>> items;
    Equipment& equipment;  // referenciaa

    // USA un consumible (desp lo elimina del inventario)
    bool use_consumable(int item_index, Player& player);
    int find_item(const Item& item) const;

 public:
    explicit Inventory(Equipment& eq);
    ~Inventory() = default;

    Inventory(const Inventory&) = delete;
    Inventory& operator=(const Inventory&) = delete;

    bool add_item(std::unique_ptr<Item> item);

    // saca el item del inventario y lo pone en el slot de equipment
    // si es consumible, lo consume en lugar de equiparlo
    // retorna false si el item no está o viola restricción
    bool equip(Item& item, Player& player);

    // saca el item del slot y lo devuelve al inventario
    // retorna false si el slot está vacío
    bool unequip(EquipmentSlot slot);

    // saca el item del inventario (para dropearlo por ej)
    std::unique_ptr<Item> remove_item(Item& item);

    bool is_full() const;
    size_t get_size() const;
    const std::vector<std::unique_ptr<Item>>& get_items() const;
};

#endif
