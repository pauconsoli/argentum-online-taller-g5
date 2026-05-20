#ifndef POTION_H
#define POTION_H

#include "server/game/items/item.h"

enum class PotionType {
    HEALTH,
    MANA
};

class Potion : public Item {
private:
    PotionType type;

    int restore;

public:
    Potion(const std::string& name, PotionType type, int restore);

    ~Potion() override;

    PotionType get_type() const;

    int get_restore() const;

    std::optional<EquipmentSlot> get_slot() const override;

    void use(Player& player) override;
};

#endif