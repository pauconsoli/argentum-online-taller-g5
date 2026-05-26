#ifndef CONSUMABLE_ITEM_H
#define CONSUMABLE_ITEM_H

#include <string>

#include "server/game/items/item.h"

enum class ConsumableType { HEALTH, MANA };

class ConsumableItem: public Item {
 private:
    ConsumableType type;

    int restore;

 public:
    ConsumableItem(const std::string& name, ConsumableType type, int restore);

    ~ConsumableItem() override;

    ConsumableType get_type() const;

    int get_restore() const;

    std::optional<EquipmentSlot> get_slot() const override;

    void use(Player& player) override;
};

#endif
