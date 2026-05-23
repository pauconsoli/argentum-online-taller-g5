#ifndef INVENTORY_H
#define INVENTORY_H
#include <vector>
#include <memory>

#include "server/game/items/item.h"
#include "server/game/equipment.h"

#define MAX_INVENTORY_ITEMS 20  // esto despues hay que sacarlo a un config o algo asi

class Player;

class Inventory {
private:
    std::vector<std::unique_ptr<Item>> items;
    Equipment equipment;
    static constexpr size_t MAX_ITEMS = MAX_INVENTORY_ITEMS;
    
    int find_item(Item& item);

public:
    Inventory();
    ~Inventory() = default;
    
    Inventory(const Inventory&) = delete;
    Inventory& operator=(const Inventory&) = delete;
    
    bool add_item(std::unique_ptr<Item> item);
    
    bool equip(Item& item);
    
    bool unequip(EquipmentSlot slot);
    
    bool use_consumable(Item& item, Player& player);
    
    bool remove_item(Item& item);
    
    Item* get_equipped(EquipmentSlot slot) const;
    const std::vector<std::unique_ptr<Item>>& get_items() const;
    size_t get_size() const;
    bool is_full() const;
    
    bool contains_item(Item& item) const;
};

#endif
