#include "server_update_handler.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "client_map.h"
#include "common/attack_result.h"

static constexpr int kMaxAudioVolume = 128;  // SDL_mixer MIX_MAX_VOLUME

ServerUpdateHandler::ServerUpdateHandler(GameState& s, Notifier& n): state(s), notifier(n) {}

void ServerUpdateHandler::apply_pending(UpdateQueue& queue, ClientMap& map, int tile_w,
                                        int tile_h) {
    std::unique_ptr<GameUpdate> update;
    while (queue.try_pop(update)) {
        switch (update->get_type()) {
            case UpdateType::ERROR:
                on_error(static_cast<const ErrorUpdate&>(*update));
                break;
            case UpdateType::SNAPSHOT:
                on_snapshot(static_cast<const SnapshotUpdate&>(*update), map, tile_w, tile_h);
                break;
            case UpdateType::PLAYER_LEFT:
                on_player_left(static_cast<const PlayerLeftUpdate&>(*update));
                break;
            case UpdateType::WORLD_MAP:
                on_world_map(static_cast<const WorldMapUpdate&>(*update), map);
                break;
            case UpdateType::INVENTORY:
                on_inventory(static_cast<const InventoryUpdate&>(*update));
                break;
            case UpdateType::ATTACKED:
                on_attack(static_cast<const AttackUpdate&>(*update));
                break;
            case UpdateType::DEATH:
                on_death(static_cast<const DeathUpdate&>(*update), tile_w, tile_h);
                break;
            case UpdateType::CHAT_MSG:
                on_chat(static_cast<const ChatMsgUpdate&>(*update));
                break;
            case UpdateType::CATALOG:
                on_catalog(static_cast<const CatalogUpdate&>(*update));
                break;
            case UpdateType::CLAN_RESULT:
                on_clan_result(static_cast<const ClanResultUpdate&>(*update));
                break;
            case UpdateType::CLAN_REVIEW:
                on_clan_review(static_cast<const ClanReviewUpdate&>(*update));
                break;
            case UpdateType::SYSTEM_MSG:
                on_system_msg(static_cast<const SystemMsgUpdate&>(*update));
                break;
            case UpdateType::NPC_INTERACT:
                on_interact(static_cast<const NpcInteractUpdate&>(*update));
                break;
            default:
                break;
        }
    }
}

void ServerUpdateHandler::on_error(const ErrorUpdate& eu) {
    const auto& d = eu.detail;
    if (d.find("move_player") != std::string::npos || d.find("mover") != std::string::npos)
        return;
    notifier.message(d);
}

void ServerUpdateHandler::on_snapshot(const SnapshotUpdate& snap, ClientMap& /*map*/, int tile_w,
                                      int tile_h) {
    state.apply_snapshot(snap, tile_w, tile_h);
}

void ServerUpdateHandler::on_player_left(const PlayerLeftUpdate& pu) {
    state.remove_player(pu.player_id);
    if (pu.clan_id != 0 && pu.clan_id == state.clan_id())
        notifier.message("[Clan] " + pu.nick + " se ha desconectado.");
}

void ServerUpdateHandler::on_world_map(const WorldMapUpdate& mu, ClientMap& client_map) {
    std::vector<MapCell> map_cells;
    map_cells.reserve(mu.cells.size());
    std::transform(mu.cells.begin(), mu.cells.end(), std::back_inserter(map_cells),
                   [](const auto& c) {
                       return MapCell{static_cast<TerrainType>(c.terrain_type), c.blocking};
                   });
    client_map = ClientMap(mu.width, mu.height, std::move(map_cells));
}

void ServerUpdateHandler::on_inventory(const InventoryUpdate& iu) {
    state.apply_inventory(iu);
}

void ServerUpdateHandler::on_attack(const AttackUpdate& au) {
    const AttackResult& result = au.get_result();
    if (result.evaded) {
        notifier.message("Ataque esquivado");
    } else if (result.attacker_id == state.player_id()) {
        std::string target_name;
        auto player_it = state.players().find(result.target_id);
        if (player_it != state.players().end()) {
            target_name = player_it->second.nick;
        } else {
            auto npc_it = state.npcs().find(result.target_id);
            if (npc_it != state.npcs().end())
                target_name = npc_it->second.name;
        }
        if (result.is_healing) {
            notifier.message(result.weapon_or_spell_name + ": curaste a " + target_name + " por " +
                             std::to_string(result.heal_amount) + " puntos");
        } else if (!result.target_died) {
            notifier.message(result.weapon_or_spell_name + ": causaste " +
                             std::to_string(result.damage) + " de daño a " + target_name);
        }
    } else if (result.target_id == state.player_id()) {
        if (result.is_healing) {
            std::string healer_name;
            auto player_it = state.players().find(result.attacker_id);
            if (player_it != state.players().end()) {
                healer_name = player_it->second.nick;
            } else {
                auto npc_it = state.npcs().find(result.attacker_id);
                if (npc_it != state.npcs().end())
                    healer_name = npc_it->second.name;
            }
            notifier.message(healer_name + " te curó " + std::to_string(result.heal_amount) +
                             " puntos de vida");
        } else if (!result.target_died) {
            notifier.message("Recibiste " + std::to_string(result.damage) + " de daño");
        }
    } else if (result.damage > 0 && result.target_clan_id != 0 &&
               result.target_clan_id == state.clan_id()) {
        std::string victim_name = "un compañero";
        auto vit = state.players().find(result.target_id);
        if (vit != state.players().end())
            victim_name = vit->second.nick;
        std::string attacker_name = "Alguien";
        auto ait = state.players().find(result.attacker_id);
        if (ait != state.players().end()) {
            attacker_name = ait->second.nick;
        } else {
            auto nit = state.npcs().find(result.attacker_id);
            if (nit != state.npcs().end())
                attacker_name = nit->second.name;
        }
        notifier.message("[Clan] ¡" + victim_name + " está siendo atacado por " + attacker_name +
                         "!");
    }
    play_attack_sound(result);
}

void ServerUpdateHandler::play_attack_sound(const AttackResult& result) {
    switch (result.type) {
        case AttackType::NORMAL:
            if (!result.evaded && result.damage > 0)
                notifier.play("melee_hit");
            break;
        case AttackType::RANGED:
            notifier.play("ranged_attack");
            break;
        case AttackType::MAGIC:
            if (!result.is_healing) {
                std::string lname = result.weapon_or_spell_name;
                std::transform(lname.begin(), lname.end(), lname.begin(),
                               [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                if (lname.find("explos") != std::string::npos)
                    notifier.play("explosion");
            }
            break;
    }
}

void ServerUpdateHandler::on_death(const DeathUpdate& du, int tile_w, int tile_h) {
    if (du.get_dead_id() == state.player_id()) {
        notifier.message("Moriste. Dirigite al sacerdote para resucitar");
        notifier.play("death");
        return;
    }

    int vol = kMaxAudioVolume;
    uint32_t dead_id = du.get_dead_id();
    int dead_tx = -1, dead_ty = -1;

    auto pit = state.players().find(dead_id);
    if (pit != state.players().end()) {
        const auto& snap = pit->second;
        dead_tx = snap.x;
        dead_ty = snap.y;
        if (du.get_killer_id() == state.player_id())
            notifier.message("Mataste a " + snap.nick);
        else
            notifier.message("Un jugador murio en combate");
    } else {
        auto nit = state.npcs().find(dead_id);
        if (nit != state.npcs().end()) {
            const auto& snap = nit->second;
            dead_tx = snap.x;
            dead_ty = snap.y;
            if (du.get_killer_id() == state.player_id())
                notifier.message("Mataste a " + snap.name);
            else
                notifier.message("Un NPC murió en combate");
        }
    }

    if (dead_tx >= 0) {
        int dx = dead_tx - state.player_x() / tile_w;
        int dy = dead_ty - state.player_y() / tile_h;
        float dist = std::sqrt(static_cast<float>(dx * dx + dy * dy));
        constexpr float kMaxDist = 20.0f;
        vol = (dist >= kMaxDist) ? 0 : static_cast<int>((1.0f - dist / kMaxDist) * kMaxAudioVolume);
    }
    notifier.play("death", vol);
}

void ServerUpdateHandler::on_chat(const ChatMsgUpdate& msg) {
    if (msg.is_private) {
        notifier.message("[Privado] " + msg.sender_nick + ": " + msg.text);
    } else {
        notifier.message(msg.sender_nick + ": " + msg.text);
    }
}

void ServerUpdateHandler::on_catalog(const CatalogUpdate& cat) {
    const auto& items = cat.get_catalog();
    if (items.empty()) {
        notifier.message("No hay items disponibles");
        return;
    }
    notifier.message("--- Catálogo ---");
    for (const auto& name : items) notifier.message(name);
    if (cat.get_gold_in_bank() > 0)
        notifier.message("Oro en banco: " + std::to_string(cat.get_gold_in_bank()));
}

void ServerUpdateHandler::on_clan_result(const ClanResultUpdate& cu) {
    const ClanResult r = cu.get_result();
    const ClanAction action = cu.get_action();

    if (r.status != ClanActionStatus::SUCCESS) {
        switch (r.status) {
            case ClanActionStatus::LEVEL_TOO_LOW:
                notifier.message("[Clan] Necesitás nivel 6 o más para fundar un clan.");
                break;
            case ClanActionStatus::ALREADY_IN_CLAN:
                notifier.message("[Clan] Ya pertenecés a un clan.");
                break;
            case ClanActionStatus::NAME_TAKEN:
                notifier.message("[Clan] Ya existe un clan con ese nombre.");
                break;
            case ClanActionStatus::NAME_EMPTY:
                notifier.message("[Clan] El nombre del clan no puede estar vacío.");
                break;
            case ClanActionStatus::CLAN_NOT_FOUND:
                notifier.message("[Clan] No existe ese clan.");
                break;
            case ClanActionStatus::NOT_FOUNDER:
                notifier.message("[Clan] Solo el fundador puede hacer eso.");
                break;
            case ClanActionStatus::NOT_IN_CLAN:
                notifier.message("[Clan] No pertenecés a ningún clan.");
                break;
            case ClanActionStatus::NOT_PENDING:
                notifier.message("[Clan] Ese jugador no tiene una solicitud pendiente.");
                break;
            case ClanActionStatus::CLAN_FULL:
                notifier.message("[Clan] El clan está lleno (máximo 16 miembros).");
                break;
            case ClanActionStatus::TARGET_OFFLINE:
                notifier.message("[Clan] Ese jugador no está conectado.");
                break;
            case ClanActionStatus::TARGET_IS_SELF:
                notifier.message("[Clan] No podés aplicar eso a vos mismo.");
                break;
            case ClanActionStatus::FOUNDER_CANNOT_LEAVE:
                notifier.message("[Clan] El fundador no puede abandonar el clan.");
                break;
            case ClanActionStatus::PLAYER_BANNED:
                notifier.message("[Clan] Estás baneado de este clan.");
                break;
            case ClanActionStatus::INTERNAL_ERROR:
            default:
                notifier.message("[Clan] Error interno del servidor.");
                break;
        }
        return;
    }

    const bool i_am_target = (r.other_player_id != 0 && r.other_player_id == state.player_id());

    switch (action) {
        case ClanAction::FOUND:
            notifier.message("[Clan] Fundaste el clan '" + r.clan_name + "'.");
            break;
        case ClanAction::JOIN_REQUEST:
            if (i_am_target) {
                notifier.message("[Clan] " + r.actor_nick +
                                 " quiere unirse a tu clan. Usá /clan-aceptar o /clan-rechazar.");
            } else {
                notifier.message("[Clan] Tu solicitud para unirte a '" + r.clan_name +
                                 "' fue enviada.");
            }
            break;
        case ClanAction::ACCEPT:
            if (i_am_target) {
                notifier.message("[Clan] Fuiste aceptado en el clan '" + r.clan_name + "'.");
            } else {
                notifier.message("[Clan] Aceptaste a " + r.other_nick + " en el clan.");
            }
            break;
        case ClanAction::REJECT:
            if (i_am_target) {
                notifier.message("[Clan] Tu solicitud al clan '" + r.clan_name +
                                 "' fue rechazada.");
            } else {
                notifier.message("[Clan] Rechazaste la solicitud de " + r.other_nick + ".");
            }
            break;
        case ClanAction::BAN:
            if (i_am_target) {
                notifier.message("[Clan] Fuiste baneado del clan '" + r.clan_name + "'.");
            } else {
                notifier.message("[Clan] Baneaste a " + r.other_nick + " del clan.");
            }
            break;
        case ClanAction::KICK:
            if (i_am_target) {
                notifier.message("[Clan] Fuiste expulsado del clan '" + r.clan_name + "'.");
            } else {
                notifier.message("[Clan] Expulsaste a " + r.other_nick + " del clan.");
            }
            break;
        case ClanAction::LEAVE:
            notifier.message("[Clan] Abandonaste el clan.");
            break;
        case ClanAction::REVIEW:
            break;
        default:
            break;
    }
}

void ServerUpdateHandler::on_clan_review(const ClanReviewUpdate& cru) {
    notifier.show_clan_review(cru);
}

void ServerUpdateHandler::on_system_msg(const SystemMsgUpdate& su) {
    notifier.message(su.get_message());
}

void ServerUpdateHandler::on_interact(const NpcInteractUpdate& update) {
    const InteractResult& result = update.get_result();
    const NPCInteraction type = update.get_npc_interaction_type();

    if (result.status != InteractStatus::SUCCESS) {
        show_interact_error(result.status);
        return;
    }

    switch (type) {
        case NPCInteraction::BUY:
            notifier.message("Compraste " + result.item_name + " por " +
                             std::to_string(result.gold_amount) + " oro");
            break;
        case NPCInteraction::SELL:
            notifier.message("Vendiste " + result.item_name + " por " +
                             std::to_string(result.gold_amount) + " oro");
            break;
        case NPCInteraction::HEAL:
            notifier.message("Fuiste curado");
            break;
        case NPCInteraction::RESURRECT:
            notifier.message("Fuiste resucitado");
            break;
        case NPCInteraction::LIST:
            break;
        case NPCInteraction::DEPOSIT_GOLD:
            notifier.message("Depositaste " + std::to_string(result.gold_amount) +
                             " oro en el banco");
            break;
        case NPCInteraction::WITHDRAW_GOLD:
            notifier.message("Retiraste " + std::to_string(result.gold_amount) + " oro del banco");
            break;
        case NPCInteraction::DEPOSIT_ITEM:
            notifier.message("Depositaste " + result.item_name + " en el banco");
            break;
        case NPCInteraction::WITHDRAW_ITEM:
            notifier.message("Retiraste " + result.item_name + " del banco");
            break;
        default:
            break;
    }
}

void ServerUpdateHandler::show_interact_error(InteractStatus status) {
    switch (status) {
        case InteractStatus::INSUFFICIENT_GOLD:
            notifier.message("No tenés oro suficiente");
            break;
        case InteractStatus::INVENTORY_FULL:
            notifier.message("Tu inventario está lleno");
            break;
        case InteractStatus::ITEM_NOT_FOUND:
            notifier.message("Ese item no existe");
            break;
        case InteractStatus::PLAYER_DEAD:
            notifier.message("No podés hacer eso mientras estás muerto");
            break;
        case InteractStatus::OUT_OF_RANGE:
            notifier.message("Estás muy lejos del NPC");
            break;
        case InteractStatus::NOT_ALLOWED:
            notifier.message("Ese NPC no puede hacer eso");
            break;
        case InteractStatus::PLAYER_NOT_DEAD:
            notifier.message("No podés resucitar estando vivo");
            break;
        case InteractStatus::ALREADY_FULL:
            notifier.message("El banco está lleno");
            break;
        case InteractStatus::INVALID_AMOUNT:
            notifier.message("Cantidad inválida");
            break;
        case InteractStatus::INVALID_TARGET:
            notifier.message("NPC no válido");
            break;
        default:
            break;
    }
}
