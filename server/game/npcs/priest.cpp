#include "server/game/npcs/priest.h"

#include "server/game/player.h"

Priest::Priest(uint32_t id, const Position& pos): CityNPC(id, "Sacerdote", pos) {}

InteractResult Priest::on_heal(Player& player) {
    if (player.is_dead()) {
        return InteractResult{InteractStatus::PLAYER_DEAD};
    }

    bool hp_full = player.get_current_hp() == player.get_max_hp();
    bool mana_full = player.get_current_mana() == player.get_max_mana();

    if (hp_full && mana_full) {
        return InteractResult{InteractStatus::ALREADY_FULL};
    }

    player.heal(player.get_max_hp());
    player.restore_mana(player.get_max_mana());

    return InteractResult{InteractStatus::SUCCESS};
}

InteractResult Priest::on_resurrect(Player& player, Bank& /*bank*/) {
    if (!player.is_dead()) {
        return InteractResult{InteractStatus::PLAYER_NOT_DEAD};
    }

    player.resurrect();

    return InteractResult{InteractStatus::SUCCESS};
}

InteractResult Priest::on_buy(const std::string& item_name, Player& player) {
    if (player.is_dead()) {
        return InteractResult{InteractStatus::PLAYER_DEAD};
    }

    // TODO(Pau): Validar que el item (item_name) sea una poción, báculo o vara.
    // TODO(Pau): Obtener el precio del item (ej. desde ItemRegistry).
    // TODO(Pau): Verificar saldo con player.get_gold() y descontar con player.remove_gold()
    // TODO(Pau): Verificar espacio y agregarlo con player.get_inventory().add_item()

    return InteractResult{InteractStatus::SUCCESS, item_name, 0};
}

InteractResult Priest::on_list(Player& player) {
    if (player.is_dead()) {
        return InteractResult{InteractStatus::PLAYER_DEAD};
    }

    // TODO(Pau): Enviar al jugador la lista de los ítems en venta por el sacerdote
    // (Pociones, varas y báculos).

    return InteractResult{InteractStatus::SUCCESS};
}
