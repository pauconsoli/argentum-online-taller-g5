#include "server/game/npcs/hostile_npc.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "server/game/game_config.h"
#include "server/game/game_formulas.h"
#include "server/game/items/item_registry.h"
#include "server/game/player.h"

HostileNPC::HostileNPC(uint32_t id, const std::string& name, NPCType npc_type, int level,
                       int max_hp, int defense, int agility, int min_damage, int max_damage,
                       int attack_range, const std::vector<ZoneType>& allowed_zones,
                       const Position& pos):
    NPC(id, name, npc_type, level, max_hp, defense, pos),
    agility(agility),
    min_damage(min_damage),
    max_damage(max_damage),
    attack_range(attack_range),
    allowed_zones(allowed_zones) {}

int HostileNPC::get_agility() const {
    return agility;
}

bool HostileNPC::can_enter_zone_type(ZoneType zone_type) const {
    return std::any_of(allowed_zones.begin(), allowed_zones.end(),
                       [zone_type](ZoneType allowed) { return allowed == zone_type; });
}

int HostileNPC::calculate_base_damage() const {
    return GameFormulas::calculate_base_damage_in_range(min_damage, max_damage);
}

NPCBehavior HostileNPC::update(float time, const std::vector<Player*>& nearby_targets) {
    NPCBehavior behavior;
    behavior.action = NPCAction::STILL;

    action_timer += time;
    if (action_timer < GameConfig::get_instance()
                           .get_chase_cooldown()) {  // cooldown de 0.5s para no ser inmatables
        return behavior;
    }

    if (nearby_targets.empty()) {
        return behavior;
    }

    Player* closest = nullptr;
    int min_dist_sq = (attack_range * attack_range) + 1;

    for (Player* p : nearby_targets) {
        if (p->is_dead())
            continue;

        int dx = p->get_position().x - position.x;
        int dy = p->get_position().y - position.y;
        int dist_sq = (dx * dx) + (dy * dy);

        if (dist_sq < min_dist_sq) {
            min_dist_sq = dist_sq;
            closest = p;
        }
    }

    if (!closest) {
        return behavior;
    }

    int dx = closest->get_position().x - position.x;
    int dy = closest->get_position().y - position.y;
    if (std::abs(dx) <= 1 && std::abs(dy) <= 1) {  // adyacente-->ataque
        behavior.action = NPCAction::ATTACK;
        behavior.target_id = closest->get_id();
        action_timer = 0.0f;
    } else {  // no adyacente pero dentro del rango-->perseguir
        behavior.action = NPCAction::CHASE;
        behavior.target_position = closest->get_position();
        action_timer = 0.0f;
    }

    return behavior;
}

Loot HostileNPC::drop_loot() {
    Loot loot;
    const GameConfig& config = GameConfig::get_instance();

    float prob = GameFormulas::get_random_float(0.0f, 1.0f);

    float p_nothing = config.get_npc_drop_nothing_prob();
    float p_gold = config.get_npc_drop_gold_prob();
    float p_potion = config.get_npc_drop_potion_prob();
    float p_item = config.get_npc_drop_item_prob();

    if (prob >= p_nothing && prob < (p_nothing + p_gold)) {
        loot.dropped_gold = GameFormulas::calculate_npc_dropped_gold(*this);
    } else if (prob >= (p_nothing + p_gold) && prob < (p_nothing + p_gold + p_potion)) {
        auto potion = ItemRegistry::get_instance().create_random_potion();
        if (potion) {
            loot.dropped_items.push_back({std::move(potion), 1, std::nullopt});
        }
    } else if (prob >= (p_nothing + p_gold + p_potion) &&
               prob < (p_nothing + p_gold + p_potion + p_item)) {
        auto random_item = ItemRegistry::get_instance().create_random_other_item();
        if (random_item) {
            loot.dropped_items.push_back({std::move(random_item), 1, std::nullopt});
        }
    }

    return loot;
}
