#include "server/game/items/potion.h"
#include "server/game/player.h"

Potion::Potion(const std::string& name, PotionType type, int restore):
        Item(name),
        type(type),
        restore(restore) {}

Potion::~Potion() = default;

PotionType Potion::get_type() const {
    return type;
}

int Potion::get_restore() const {
    return restore;
}

std::optional<EquipmentSlot> Potion::get_slot() const {
    return std::nullopt;  // no se equipa, se consume
}

void Potion::use(Player& player) {

    if (type == PotionType::HEALTH) {
        player.restore_health(restore);
    } else {
        player.restore_mana(restore);
    }
    // eliminar la pocion del inventario del jugador, pero eso lo hace el player, no el item 
    // ? 
}