#ifndef DEFENSIVE_ITEM_H
#define DEFENSIVE_ITEM_H

#include <string>

#include "server/game/items/item.h"

class DefensiveItem: public Item {
 private:
    int min_defense;
    int max_defense;

    EquipmentSlot slot;

 public:
    DefensiveItem(const std::string& name, int min_defense, int max_defense, EquipmentSlot slot);

    ~DefensiveItem() override;

    int get_min_defense() const;
    int get_max_defense() const;

    std::optional<EquipmentSlot> get_slot() const override;

    void use(Player& player) override;
};

#endif
