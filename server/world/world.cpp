#include "world.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <queue>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "common/updates/attack_update.h"
#include "server/game/clan.h"
#include "server/game/game_config.h"
#include "server/game/game_formulas.h"
#include "server/game/items/weapon.h"
#include "server/game/npcs/banker.h"
#include "server/game/npcs/hostile_npc.h"
#include "server/game/npcs/merchant.h"
#include "server/game/npcs/npc_registry.h"
#include "server/game/npcs/priest.h"
#include "server/world/dungeon.h"

World::World(int width, int height):
    map(width, height), occupied(height, std::vector<bool>(width, false)) {
    spawn_city_npcs();
    spawn_initial_hostile_npcs();
}

World::World(WorldMap world_map):
    map(std::move(world_map)),
    occupied(map.get_height(), std::vector<bool>(map.get_width(), false)) {
    spawn_city_npcs();
    spawn_initial_hostile_npcs();
}


void World::add_player(std::unique_ptr<Player> player) {
    if (!player) {
        throw std::invalid_argument("World::add_player: intento de agregar un jugador nulo");
    }

    uint32_t id = player->get_id();
    if (player_exists(id)) {
        throw std::runtime_error("World::add_player: ya existe un jugador con ese ID");
    }

    Position pos = player->get_position();  // obtengo pos

    if (!map.is_valid_position(pos)) {
        throw std::out_of_range("World::add_player: la posición de spawn es inválida");
    }

    if (map.is_position_blocked(pos)) {
        throw std::runtime_error(
            "World::add_player: la posición de spawn está bloqueada por el terreno");
    }

    if (is_position_occupied(pos)) {
        throw std::runtime_error("World::add_player: la posición de spawn está ocupada");
    }

    occupied[pos.y][pos.x] = true;    // marco la posición como ocupada
    players[id] = std::move(player);  // agrego el player al map de players
}

void World::remove_player(uint32_t player_id) {
    auto it = players.find(player_id);
    if (it != players.end()) {
        Position pos = it->second->get_position();
        occupied[pos.y][pos.x] = false;          // marco la posición como libre
        players.erase(it);                       // elimino el jugador
        pending_resurrections.erase(player_id);  // cancelo la resurrección si se desconecta
    }
}


Player* World::get_player(uint32_t player_id) {
    auto it = players.find(player_id);
    if (it != players.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::vector<Player*> World::get_players() {
    std::vector<Player*> active_players;
    active_players.reserve(players.size());
    for (auto& [id, player] : players) {
        active_players.push_back(player.get());
    }
    return active_players;
}

bool World::player_exists(uint32_t player_id) const {
    return players.find(player_id) != players.end();
}

Player* World::get_player_by_name(const std::string& name) {
    for (auto& [id, player] : players) {
        if (player->get_name() == name) {
            return player.get();
        }
    }
    return nullptr;
}

Character* World::get_character(uint32_t id) {
    if (Player* p = get_player(id)) {
        return p;
    }
    if (NPC* n = get_npc(id)) {
        return n;
    }
    return nullptr;
}

// (0,0) es la esquina superior izquierda, x aumenta hacia la derecha e y hacia abajo
Position World::calculate_destination(const Position& current, Direction direction) const {
    Position dest = current;
    switch (direction) {
        case Direction::UP:
            dest.y -= 1;
            break;
        case Direction::DOWN:
            dest.y += 1;
            break;
        case Direction::LEFT:
            dest.x -= 1;
            break;
        case Direction::RIGHT:
            dest.x += 1;
            break;
    }
    return dest;
}

bool World::is_position_occupied(const Position& position) const {
    return occupied[position.y][position.x];
}

bool World::is_in_range_for_attack(const Character* attacker, const Character* target) const {
    bool is_ranged = attacker->is_ranged_attack();
    if (!is_ranged) {
        Position attacker_pos = attacker->get_position();
        Position target_pos = target->get_position();
        int dif_x = std::abs(attacker_pos.x - target_pos.x);
        int dif_y = std::abs(attacker_pos.y - target_pos.y);
        return dif_x <= 1 && dif_y <= 1;  // estoy adyacente
    }
    return true;  // a dist, rango ilimitado (visibilidad del objetivo)
}

// devuelve la posición de spawn para un nuevo jugador, que es la posición configurada en GameConfig
// pero si está ocupada, da la siguiente posición libre más cercana da la siguiente posición libre
// más cercana
Position World::get_spawn_position() const {
    const Position base_spawn = GameConfig::get_instance().get_spawn_position();

    for (int y = base_spawn.y; y < map.get_height(); ++y) {
        int x_start = (y == base_spawn.y) ? base_spawn.x : 0;  //
        for (int x = x_start; x < map.get_width(); ++x) {
            Position spawn{x, y};
            if (!map.is_position_blocked(spawn) && !is_position_occupied(spawn)) {
                return spawn;
            }
        }
    }

    for (int y = 0; y < base_spawn.y; ++y) {
        for (int x = 0; x < map.get_width(); ++x) {
            Position spawn{x, y};
            if (!map.is_position_blocked(spawn) && !is_position_occupied(spawn)) {
                return spawn;
            }
        }
    }

    throw std::runtime_error("World::get_spawn_position: no hay posiciones libres en el mapa");
}

const std::map<Position, GroundItem>& World::get_ground_items() const {
    return ground_items;
}

std::optional<Position> World::find_closest_free_position(
    const Position& start, const std::function<bool(const Position&)>& is_free_condition) const {
    std::queue<Position> q;
    std::set<Position> visited;

    if (map.is_valid_position(start)) {
        q.push(start);
        visited.insert(start);
    }

    while (!q.empty()) {
        Position curr = q.front();
        q.pop();

        if (!map.is_position_blocked(curr) && is_free_condition(curr)) {
            return curr;
        }

        const Position neighbors[] = {
            {curr.x, curr.y - 1}, {curr.x, curr.y + 1}, {curr.x - 1, curr.y}, {curr.x + 1, curr.y}};

        for (const Position& neighbor : neighbors) {
            if (map.is_valid_position(neighbor) && visited.insert(neighbor).second) {
                q.push(neighbor);
            }
        }
    }
    return std::nullopt;
}

// no considera si esta ocupada porque el jugador se para sobre los items del suelo
std::optional<Position> World::find_closest_free_ground(const Position& start) const {
    return find_closest_free_position(start, [this](const Position& pos) {
        return ground_items.find(pos) == ground_items.end();
    });
}

std::optional<Position> World::find_closest_unoccupied_position(const Position& start) const {
    return find_closest_free_position(
        start, [this](const Position& pos) { return !is_position_occupied(pos); });
}

void World::drop_loot_in_world(const Position& center, Loot loot) {
    if (loot.dropped_gold > 0) {
        if (auto free_pos = find_closest_free_ground(center)) {
            ground_items[*free_pos] = GroundItem{loot.dropped_gold, nullptr, 0};
        }
    }

    for (auto& slot : loot.dropped_items) {
        if (auto free_pos = find_closest_free_ground(center)) {
            ground_items[*free_pos] = GroundItem{0, std::move(slot.item), slot.quantity};
        } else {
            break;
        }
    }
}

void World::add_ground_item(const Position& pos, GroundItem item) {
    if (!map.is_valid_position(pos)) {
        return;
    }

    Position free_pos = pos;
    if (map.is_position_blocked(pos) || ground_items.find(pos) != ground_items.end()) {
        if (auto maybe_pos = find_closest_free_ground(pos)) {
            free_pos = *maybe_pos;
        } else {
            return;
        }
    }
    ground_items[free_pos] = std::move(item);
}

AttackStatus World::validate_attack_conditions(const Character* attacker,
                                               const Character* target) const {
    if (attacker->is_dead() || target->is_dead()) {
        return AttackStatus::DEAD;
    }

    if (attacker->get_id() == target->get_id()) {
        return AttackStatus::INVALID_TARGET;
    }

    if (!target->validate_attack_from(attacker->get_level())) {
        return AttackStatus::INVALID_TARGET;
    }

    // zonas seguras
    if (map.is_safe(attacker->get_position()) || map.is_safe(target->get_position())) {
        return AttackStatus::INVALID_TARGET;
    }

    // mismo clan
    uint32_t attacker_clan = attacker->get_clan_id();
    uint32_t target_clan = target->get_clan_id();
    if (attacker_clan != 0 && attacker_clan == target_clan) {
        return AttackStatus::INVALID_TARGET;
    }

    if (!is_in_range_for_attack(attacker, target)) {
        return AttackStatus::OUT_OF_RANGE;
    }

    return AttackStatus::SUCCESS;
}

void World::handle_target_death(Character* attacker, Character* target) {
    int bonus_exp = GameFormulas::calculate_kill_experience_gain(*attacker, *target);
    attacker->add_experience(bonus_exp);

    Loot loot = target->drop_loot();
    drop_loot_in_world(target->get_position(), std::move(loot));
}

int World::get_nearby_clan_members_count(const Character* character, int range) const {
    uint32_t clan_id = character->get_clan_id();
    if (clan_id == 0)
        return 0;

    auto nearby = get_players_near(character->get_position(), range);
    return static_cast<int>(
        std::count_if(nearby.begin(), nearby.end(), [character, clan_id](Player* other) {
            return other->get_id() != character->get_id() && other->get_clan_id() == clan_id;
        }));
}


int World::handle_successful_attack(Character* attacker, Character* target, int damage) {
    int defense = target->get_defense();

    // bonus de clan
    int clan_count =
        get_nearby_clan_members_count(target, GameConfig::get_instance().get_bonus_range());
    if (clan_count > 0) {
        defense +=
            static_cast<int>(defense * GameConfig::get_instance().get_bonus_factor() * clan_count);
    }

    int real_damage = std::max(0, damage - defense);
    target->receive_damage(real_damage);

    int exp = GameFormulas::calculate_attack_experience_gain(*attacker, *target, real_damage);
    attacker->add_experience(exp);

    return real_damage;
}

bool World::move_character(uint32_t character_id, Direction direction) {
    Character* character = get_character(character_id);
    if (!character) {
        return false;
    }

    if (pending_resurrections.count(character_id)) {
        return false;  // el fantasma está inmovilizado esperando la resurrección
    }

    Position current = character->get_position();
    Position next = calculate_destination(current, direction);

    if (!map.is_valid_position(next)) {
        return false;
    }

    if (map.is_position_blocked(next)) {  // si la celda destino es bloqueante, no se mueve
        return false;
    }

    if (map.is_safe(next) && !character->can_enter_safe_zones()) {
        return false;
    }

    if (is_position_occupied(next)) {
        return false;
    }

    // si llegamos acá, la posición destino es válida, entonces se puede mover
    occupied[current.y][current.x] = false;
    character->set_position(next);
    occupied[next.y][next.x] = true;
    return true;
}

bool World::teleport_player(uint32_t player_id, const Position& dest) {
    Player* player = get_player(player_id);
    if (!player)
        return false;

    auto maybe_dest = find_closest_unoccupied_position(dest);
    if (!maybe_dest)
        return false;

    Position final_dest = *maybe_dest;
    Position current = player->get_position();
    occupied[current.y][current.x] = false;
    player->set_position(final_dest);
    occupied[final_dest.y][final_dest.x] = true;
    return true;
}

MeditateStatus World::meditate(uint32_t player_id) {
    Player* player = get_player(player_id);
    if (!player || player->is_dead()) {
        return MeditateStatus::PLAYER_DEAD;
    }
    player->start_meditating();
    return player->is_meditating() ? MeditateStatus::SUCCESS : MeditateStatus::NO_MANA;
}

bool World::start_resurrection(uint32_t player_id) {
    Player* player = get_player(player_id);
    if (!player || !player->is_dead()) {
        return false;
    }

    const City* closest_city = map.get_closest_city(player->get_position());
    if (!closest_city) {
        return false;  // no hay ciudades
    }

    if (pending_resurrections.count(player_id)) {
        return false;  // ya está en proceso de resurrección
    }

    Position priest_pos = closest_city->get_priest_position();
    Position player_pos = player->get_position();

    // calculo distancia entre el jugador y el sacerdote para determinar el tiempo de espera antes
    // de la resurrección
    int dx = priest_pos.x - player_pos.x;
    int dy = priest_pos.y - player_pos.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    float wait_time = distance * GameConfig::get_instance().get_resurrect_still_factor();

    pending_resurrections[player_id] = {wait_time, priest_pos};
    return true;
}

void World::add_npc(std::unique_ptr<NPC> npc) {
    if (!npc)
        return;
    Position pos = npc->get_position();
    if (!map.is_valid_position(pos) || map.is_position_blocked(pos) || is_position_occupied(pos))
        return;
    occupied[pos.y][pos.x] = true;
    npcs[npc->get_id()] = std::move(npc);
}

NPC* World::get_npc(uint32_t npc_id) {
    auto it = npcs.find(npc_id);
    return (it != npcs.end()) ? it->second.get() : nullptr;
}

std::vector<NPC*> World::get_npcs() {
    std::vector<NPC*> active_npcs;
    active_npcs.reserve(npcs.size());
    for (auto& [id, npc] : npcs) {
        active_npcs.push_back(npc.get());
    }
    return active_npcs;
}

Bank& World::get_bank() {
    return bank;
}

std::vector<Player*> World::get_players_near(const Position& pos, float range) const {
    std::vector<Player*> nearby;
    for (auto& [id, player] : players) {
        if (player->is_dead())
            continue;
        int dx = player->get_position().x - pos.x;
        int dy = player->get_position().y - pos.y;
        if ((dx * dx + dy * dy) <= static_cast<int>(range * range))
            nearby.push_back(player.get());
    }
    return nearby;
}

InteractResult World::interact_with_npc(uint32_t player_id, uint32_t npc_id, NPCInteraction type,
                                        const std::string& arg, int amount) {
    Player* player = get_player(player_id);
    NPC* npc = get_npc(npc_id);

    if (!player || !npc)
        return InteractResult{InteractStatus::INVALID_TARGET};

    // el jugador tiene que estar adyacente al NPC
    const Position& pp = player->get_position();
    const Position& np = npc->get_position();
    if (std::abs(pp.x - np.x) > 1 || std::abs(pp.y - np.y) > 1)
        return InteractResult{InteractStatus::OUT_OF_RANGE};

    return npc->interact(type, arg, amount, *player, bank);
}

// SOLO PARA TESTS
void World::set_cell(const Position& pos, const Cell& cell) {
    map.set_cell(pos, cell);
}


Clan* World::create_clan(const std::string& name, uint32_t founder_id) {
    if (get_clan_by_name(name) != nullptr) {
        return nullptr;  // Ya existe un clan con ese nombre
    }
    uint32_t new_id = next_clan_id++;
    auto clan = std::make_unique<Clan>(new_id, name, founder_id);
    Clan* ptr = clan.get();
    clans_by_id[new_id] = std::move(clan);
    return ptr;
}

Clan* World::get_clan(uint32_t clan_id) {
    auto it = clans_by_id.find(clan_id);
    return (it != clans_by_id.end()) ? it->second.get() : nullptr;
}

Clan* World::get_clan_by_name(const std::string& name) {
    for (const auto& [id, clan] : clans_by_id) {
        if (clan->get_name() == name) {
            return clan.get();
        }
    }
    return nullptr;
}


bool World::resolve_founder_and_target(uint32_t founder_id, const std::string& target_nick,
                                       Player** founder_out, Clan** clan_out, Player** target_out,
                                       ClanActionStatus* fail_status) {
    Player* founder = get_player(founder_id);
    Clan* clan = founder ? get_clan(founder->get_clan_id()) : nullptr;
    if (!clan) {
        *fail_status = ClanActionStatus::NOT_IN_CLAN;
        return false;
    }

    Player* target = get_player_by_name(target_nick);
    if (!target) {
        *fail_status = ClanActionStatus::TARGET_OFFLINE;
        return false;
    }

    *founder_out = founder;
    *clan_out = clan;
    *target_out = target;
    return true;
}

ClanResult World::found_clan(uint32_t founder_id, const std::string& clan_name) {
    Player* founder = get_player(founder_id);
    if (!founder) {
        return ClanResult(ClanActionStatus::INTERNAL_ERROR, clan_name);
    }
    if (clan_name.empty()) {
        return ClanResult(ClanActionStatus::NAME_EMPTY, clan_name);
    }
    int min_level = GameConfig::get_instance().get_clan_min_level_to_found();
    if (founder->get_level() < min_level) {
        return ClanResult(ClanActionStatus::LEVEL_TOO_LOW, clan_name);
    }
    if (founder->get_clan_id() != 0) {
        return ClanResult(ClanActionStatus::ALREADY_IN_CLAN, clan_name);
    }

    Clan* clan = create_clan(clan_name, founder_id);
    if (!clan) {
        return ClanResult(ClanActionStatus::NAME_TAKEN, clan_name);
    }
    founder->set_clan_id(clan->get_id());
    return ClanResult(ClanActionStatus::SUCCESS, clan_name, founder->get_name());
}

ClanResult World::request_join_clan(uint32_t player_id, const std::string& clan_name) {
    Player* player = get_player(player_id);
    if (!player) {
        return ClanResult(ClanActionStatus::INTERNAL_ERROR, clan_name);
    }
    if (player->get_clan_id() != 0) {
        return ClanResult(ClanActionStatus::ALREADY_IN_CLAN, clan_name);
    }

    Clan* clan = get_clan_by_name(clan_name);
    if (!clan) {
        return ClanResult(ClanActionStatus::CLAN_NOT_FOUND, clan_name);
    }
    if (!clan->request_join(player_id)) {
        ClanActionStatus status =
            clan->is_full() ? ClanActionStatus::CLAN_FULL : ClanActionStatus::INTERNAL_ERROR;
        return ClanResult(status, clan->get_name(), player->get_name());
    }

    ClanResult result(ClanActionStatus::SUCCESS, clan->get_name(), player->get_name());
    Player* founder = get_player(clan->get_founder_id());
    if (founder) {
        result.other_player_id = founder->get_id();
        result.other_nick = founder->get_name();
    }
    return result;
}

ClanResult World::accept_clan_member(uint32_t founder_id, const std::string& target_nick) {
    Player* founder_ptr = nullptr;
    Clan* clan = nullptr;
    Player* target = nullptr;
    ClanActionStatus fail_status;
    if (!resolve_founder_and_target(founder_id, target_nick, &founder_ptr, &clan, &target,
                                    &fail_status)) {
        return ClanResult(fail_status, "", "", 0, target_nick);
    }
    std::string founder_nick = founder_ptr->get_name();

    if (!clan->accept(founder_id, target->get_id())) {
        ClanActionStatus status = clan->get_founder_id() != founder_id ?
                                      ClanActionStatus::NOT_FOUNDER :
                                      ClanActionStatus::NOT_PENDING;
        return ClanResult(status, clan->get_name(), founder_nick, 0, target_nick);
    }

    target->set_clan_id(clan->get_id());
    return ClanResult(ClanActionStatus::SUCCESS, clan->get_name(), founder_nick, target->get_id(),
                      target_nick);
}

ClanResult World::reject_clan_member(uint32_t founder_id, const std::string& target_nick) {
    Player* founder_ptr = nullptr;
    Clan* clan = nullptr;
    Player* target = nullptr;
    ClanActionStatus fail_status;
    if (!resolve_founder_and_target(founder_id, target_nick, &founder_ptr, &clan, &target,
                                    &fail_status)) {
        return ClanResult(fail_status, "", "", 0, target_nick);
    }
    std::string founder_nick = founder_ptr->get_name();

    if (!clan->reject(founder_id, target->get_id())) {
        ClanActionStatus status = clan->get_founder_id() != founder_id ?
                                      ClanActionStatus::NOT_FOUNDER :
                                      ClanActionStatus::NOT_PENDING;
        return ClanResult(status, clan->get_name(), founder_nick, 0, target_nick);
    }

    return ClanResult(ClanActionStatus::SUCCESS, clan->get_name(), founder_nick, target->get_id(),
                      target_nick);
}

ClanResult World::ban_clan_member(uint32_t founder_id, const std::string& target_nick) {
    Player* founder_ptr = nullptr;
    Clan* clan = nullptr;
    Player* target = nullptr;
    ClanActionStatus fail_status;
    if (!resolve_founder_and_target(founder_id, target_nick, &founder_ptr, &clan, &target,
                                    &fail_status)) {
        return ClanResult(fail_status, "", "", 0, target_nick);
    }
    std::string founder_nick = founder_ptr->get_name();

    bool was_member = clan->is_member(target->get_id());
    if (!clan->ban(founder_id, target->get_id())) {
        ClanActionStatus status = clan->get_founder_id() != founder_id ?
                                      ClanActionStatus::NOT_FOUNDER :
                                      ClanActionStatus::TARGET_IS_SELF;
        return ClanResult(status, clan->get_name(), founder_nick, 0, target_nick);
    }

    if (was_member) {
        target->set_clan_id(0);
    }
    return ClanResult(ClanActionStatus::SUCCESS, clan->get_name(), founder_nick, target->get_id(),
                      target_nick);
}

ClanResult World::kick_clan_member(uint32_t founder_id, const std::string& target_nick) {
    Player* founder_ptr = nullptr;
    Clan* clan = nullptr;
    Player* target = nullptr;
    ClanActionStatus fail_status;
    if (!resolve_founder_and_target(founder_id, target_nick, &founder_ptr, &clan, &target,
                                    &fail_status)) {
        return ClanResult(fail_status, "", "", 0, target_nick);
    }
    std::string founder_nick = founder_ptr->get_name();

    if (!clan->kick(founder_id, target->get_id())) {
        ClanActionStatus status = clan->get_founder_id() != founder_id ?
                                      ClanActionStatus::NOT_FOUNDER :
                                      ClanActionStatus::NOT_PENDING;
        return ClanResult(status, clan->get_name(), founder_nick, 0, target_nick);
    }

    target->set_clan_id(0);
    return ClanResult(ClanActionStatus::SUCCESS, clan->get_name(), founder_nick, target->get_id(),
                      target_nick);
}

ClanResult World::leave_clan(uint32_t player_id) {
    Player* player = get_player(player_id);
    Clan* clan = player ? get_clan(player->get_clan_id()) : nullptr;
    if (!clan) {
        return ClanResult(ClanActionStatus::NOT_IN_CLAN, "");
    }

    if (!clan->leave(player_id)) {
        return ClanResult(ClanActionStatus::FOUNDER_CANNOT_LEAVE, clan->get_name(),
                          player->get_name());
    }

    std::string clan_name = clan->get_name();
    std::string player_nick = player->get_name();
    player->set_clan_id(0);
    return ClanResult(ClanActionStatus::SUCCESS, clan_name, player_nick);
}

ClanReviewResult World::review_clan(uint32_t founder_id) {
    Player* founder = get_player(founder_id);
    Clan* clan = founder ? get_clan(founder->get_clan_id()) : nullptr;
    if (!clan) {
        return {ClanActionStatus::NOT_IN_CLAN, "", {}, {}};
    }
    if (clan->get_founder_id() != founder_id) {
        return {ClanActionStatus::NOT_FOUNDER, clan->get_name(), {}, {}};
    }

    ClanReviewResult result{ClanActionStatus::SUCCESS, clan->get_name(), {}, {}};
    for (uint32_t mid : clan->get_members()) {
        Player* m = get_player(mid);
        result.members.push_back(
            {mid, m ? m->get_name() : "", mid == clan->get_founder_id(), m != nullptr});
    }
    for (uint32_t pid : clan->get_pending()) {
        Player* p = get_player(pid);
        result.pending.push_back({pid, p ? p->get_name() : "", false, p != nullptr});
    }
    return result;
}


AttackResult World::attack(uint32_t attacker_id, uint32_t target_id) {
    Character* attacker = get_character(attacker_id);
    Character* target = get_character(target_id);

    if (!attacker || !target) {
        return AttackResult{
            attacker_id,        target_id, 0, false, false, false, 0, AttackStatus::INVALID_TARGET,
            AttackType::NORMAL, "",        0};
    }

    AttackType attack_type = AttackType::NORMAL;
    if (attacker->is_magic_attack()) {
        attack_type = AttackType::MAGIC;
    } else if (attacker->is_ranged_attack()) {
        attack_type = AttackType::RANGED;
    }
    std::string attack_name = attacker->get_attack_name();

    AttackStatus status = validate_attack_conditions(attacker, target);
    if (status != AttackStatus::SUCCESS) {
        return AttackResult{attacker_id, target_id, 0,           false,       false, false,
                            0,           status,    attack_type, attack_name, 0};
    }
    status = attacker->consume_attack_resources();
    if (status != AttackStatus::SUCCESS) {
        return AttackResult{attacker_id, target_id, 0,           false,       false, false,
                            0,           status,    attack_type, attack_name, 0};
    }

    int base_damage = attacker->calculate_base_damage();

    // Bonificación de ataque por compañeros de clan cercanos (ej. +10% por miembro, rango 5 tiles)
    int clan_count = get_nearby_clan_members_count(attacker, 5.0f);
    if (clan_count > 0) {
        base_damage += static_cast<int>(base_damage * 0.1f * clan_count);
    }

    bool is_critical = GameFormulas::calculate_critical_attack();

    if (is_critical) {
        base_damage *= GameConfig::get_instance().get_critical_multiplier();
    }

    bool evaded = !is_critical && GameFormulas::calculate_evasion(*target);

    int real_damage = 0;
    if (!evaded) {
        real_damage = handle_successful_attack(attacker, target, base_damage);
    }

    bool died = target->is_dead();
    if (died) {
        handle_target_death(attacker, target);
    }

    return AttackResult{attacker_id,
                        target_id,
                        real_damage,
                        evaded,
                        died,
                        false,
                        0,
                        AttackStatus::SUCCESS,
                        attack_type,
                        attack_name,
                        target->get_clan_id()};
}

AttackResult World::heal(uint32_t healer_id, uint32_t target_id) {
    Character* healer = get_character(healer_id);
    Character* target = get_character(target_id);

    if (!healer || !target) {
        return AttackResult{
            healer_id,         target_id, 0, false, false, false, 0, AttackStatus::INVALID_TARGET,
            AttackType::MAGIC, "",        0};
    }

    AttackType attack_type = AttackType::MAGIC;
    std::string attack_name = healer->get_attack_name();

    if (healer->is_dead() || target->is_dead()) {
        return AttackResult{healer_id, target_id,          0,           false,       false, false,
                            0,         AttackStatus::DEAD, attack_type, attack_name, 0};
    }

    if (!is_in_range_for_attack(healer, target)) {
        return AttackResult{healer_id,   target_id,   0, false,
                            false,       false,       0, AttackStatus::OUT_OF_RANGE,
                            attack_type, attack_name, 0};
    }

    AttackStatus status = healer->consume_attack_resources();
    if (status != AttackStatus::SUCCESS) {
        return AttackResult{healer_id, target_id, 0,           false,       false, false,
                            0,         status,    attack_type, attack_name, 0};
    }

    int potential_healing = healer->calculate_base_healing();
    int actual_healing = target->heal(potential_healing);
    return AttackResult{
        healer_id,   target_id,   0, false, false, true, actual_healing, AttackStatus::SUCCESS,
        attack_type, attack_name, 0};
}

// para agarrar item TENGO QUE PARARME ARRIBA, uso la pos
bool World::pick_up_item(uint32_t player_id) {
    Player* player = get_player(player_id);
    if (!player)
        return false;
    if (player->is_dead())
        return false;

    Position pos = player->get_position();
    auto it = ground_items.find(pos);
    if (it == ground_items.end())
        return false;

    GroundItem& ground = it->second;

    if (ground.gold > 0) {
        player->add_gold(ground.gold);
        ground_items.erase(it);
        return true;
    }

    if (!player->get_inventory().can_add_item(*ground.item)) {
        return false;
    }

    player->get_inventory().add_item(std::move(ground.item), ground.quantity);

    ground_items.erase(it);
    return true;
}


bool World::drop_item(uint32_t player_id, int slot_index) {
    Player* player = get_player(player_id);
    if (!player)
        return false;
    if (player->is_dead())
        return false;

    auto maybe_pos = find_closest_free_ground(player->get_position());
    if (!maybe_pos)
        return false;

    InventorySlot slot = player->get_inventory().pop_slot(slot_index);
    if (!slot.item)
        return false;

    ground_items[*maybe_pos] = GroundItem{0, std::move(slot.item), slot.quantity};
    return true;
}

bool World::equip_item(uint32_t player_id, int slot_index) {
    Player* player = get_player(player_id);
    if (!player)
        return false;
    if (player->is_dead())
        return false;

    const auto& inv_slots = player->get_inventory().get_slots();
    int num_slots = inv_slots.size();
    if (slot_index < 0 || slot_index >= num_slots) {
        return false;
    }
    if (!inv_slots[slot_index].item) {
        return false;
    }

    player->equip(*inv_slots[slot_index].item);
    return true;
}

void World::npc_move_towards(NPC* npc, const Position& target_pos) {
    Position curr = npc->get_position();
    int dx = target_pos.x - curr.x;
    int dy = target_pos.y - curr.y;

    Direction dir;
    if (std::abs(dx) >= std::abs(dy))
        dir = (dx > 0) ? Direction::RIGHT : Direction::LEFT;
    else
        dir = (dy > 0) ? Direction::DOWN : Direction::UP;

    npc->set_direction(dir);
    bool moved = move_character(npc->get_id(), dir);
    if (moved) {
        npc->set_moving(true);
    }
}

AttackResult World::npc_attack(NPC* npc, Character* target) {
    uint32_t attacker_id = npc->get_id();
    uint32_t target_id = target->get_id();
    std::string attack_name = npc->get_attack_name();

    int base_damage = npc->calculate_base_damage();
    bool evaded = GameFormulas::calculate_evasion(*target);

    int real_damage = 0;
    if (!evaded) {
        real_damage = handle_successful_attack(npc, target, base_damage);
    }

    bool died = target->is_dead();
    if (died) {
        handle_target_death(npc, target);
    }

    return AttackResult{attacker_id,
                        target_id,
                        real_damage,
                        evaded,
                        died,
                        false,
                        0,
                        AttackStatus::SUCCESS,
                        AttackType::NORMAL,
                        attack_name,
                        target->get_clan_id()};
}

std::optional<Position> World::find_random_spawn_position(
    const std::vector<std::string>& allowed_zones) const {
    static constexpr int MAX_ATTEMPTS =
        50;  // para evitar loops infinitos si el mapa está muy lleno o no hay zonas válidas

    for (int i = 0; i < MAX_ATTEMPTS; ++i) {
        int x = GameFormulas::get_random_int(0, map.get_width() - 1);
        int y = GameFormulas::get_random_int(0, map.get_height() - 1);
        Position pos{x, y};

        // descartar posiciones inválidas
        if (map.is_position_blocked(pos) || is_position_occupied(pos) || map.is_safe(pos))
            continue;

        const Zone* zone = map.get_zone(pos);
        bool is_dungeon = (dynamic_cast<const Dungeon*>(zone) != nullptr);

        // all no incluye dungeons (ni zonas seguras obviamente)
        // dungeon es solo mazmorras
        for (const std::string& z : allowed_zones) {
            if (z == "all" && !is_dungeon)
                return pos;
            if (z == "dungeon" && is_dungeon)
                return pos;
        }
    }
    return std::nullopt;
}

void World::try_spawn_npc() {
    int current = static_cast<int>(std::count_if(npcs.begin(), npcs.end(), [](const auto& kv) {
        return kv.second->is_hostile() && !kv.second->is_dead();
    }));

    int max_npcs = GameConfig::get_instance().get_npc_population_limit();
    if (current >= max_npcs)
        return;

    auto templates = NPCRegistry::get_instance().get_all_templates();
    if (templates.empty())
        return;

    int template_idx = GameFormulas::get_random_int(0, templates.size() - 1);
    const NPCTemplate* tpl = templates[template_idx];

    if (auto pos =
            find_random_spawn_position(tpl->zones)) {  // busco con las ZONAS PERMITIDAS de ese NPC
        auto npc = std::make_unique<HostileNPC>(
            next_npc_id++, tpl->name, tpl->npc_type, tpl->level, tpl->max_hp, tpl->defense,
            tpl->agility, tpl->min_damage, tpl->max_damage, tpl->attack_range, tpl->zones, *pos);
        add_npc(std::move(npc));
    }
}


void World::spawn_city_npcs() {
    for (const City* city : map.get_cities()) {
        auto priest =
            std::make_unique<Priest>(next_npc_id++, NPCType::PRIEST, city->get_priest_position());
        add_npc(std::move(priest));

        auto merchant = std::make_unique<Merchant>(next_npc_id++, NPCType::MERCHANT,
                                                   city->get_merchant_position());
        add_npc(std::move(merchant));

        auto banker =
            std::make_unique<Banker>(next_npc_id++, NPCType::BANKER, city->get_banker_position());
        add_npc(std::move(banker));
    }
}

void World::spawn_initial_hostile_npcs() {
    int max_npcs = GameConfig::get_instance().get_npc_population_limit();
    for (int i = 0; i < max_npcs; ++i) {
        try_spawn_npc();
    }
}

void World::update_players(float tick_seconds) {
    for (auto& [id, player] : players) {
        if (player->is_dead())
            continue;

        // HP
        float hp_regen = GameFormulas::calculate_health_recovery(*player, tick_seconds);
        player->add_partial_hp(hp_regen);
        if (player->get_partial_hp() >= 1.0f) {
            int heal_amount = static_cast<int>(player->get_partial_hp());
            player->heal(heal_amount);
            player->decrease_partial_hp(heal_amount);
        }

        // Mana (meditando o por tiempo)
        float mana_regen =
            player->is_meditating() ?
                GameFormulas::calculate_meditation_mana_recovery(*player, tick_seconds) :
                GameFormulas::calculate_time_mana_recovery(*player, tick_seconds);
        player->add_partial_mana(mana_regen);
        if (player->get_partial_mana() >= 1.0f) {
            int restore_amount = static_cast<int>(player->get_partial_mana());
            player->restore_mana(restore_amount);
            player->decrease_partial_mana(restore_amount);
        }
    }
}

void World::update_resurrections(float tick_seconds) {
    // procesar resurrecciones pendientes
    for (auto it = pending_resurrections.begin(); it != pending_resurrections.end();) {
        it->second.timer -= tick_seconds;
        if (it->second.timer <= 0.0f) {
            uint32_t pid = it->first;
            if (teleport_player(pid, it->second.destination)) {
                if (Player* p = get_player(pid)) {
                    p->resurrect();  // le restaura salud, mana y quita el estado fantasma
                }
            }
            it = pending_resurrections.erase(it);
        } else {
            ++it;
        }
    }
}

std::vector<AttackResult> World::update_npcs(float tick_seconds) {
    std::vector<AttackResult> attack_results;
    // acciones de NPCs hostiles
    for (auto it = npcs.begin(); it != npcs.end();) {
        auto& npc = it->second;

        if (npc->is_dead()) {
            Position pos = npc->get_position();
            occupied[pos.y][pos.x] = false;
            it = npcs.erase(it);  // remuevo NPC del mundo
            continue;
        }

        npc->set_moving(false);

        if (!npc->is_hostile()) {
            ++it;
            continue;
        }

        auto nearby = get_players_near(npc->get_position(), npc->get_attack_range());
        NPCBehavior behavior = npc->update(tick_seconds, nearby);

        switch (behavior.action) {
            case NPCAction::CHASE:
                npc_move_towards(npc.get(), behavior.target_position);
                break;
            case NPCAction::ATTACK: {
                Character* target = get_character(behavior.target_id);
                if (target)
                    attack_results.push_back(npc_attack(npc.get(), target));
                break;
            }
            case NPCAction::STILL:
                break;
        }
        ++it;
    }
    return attack_results;
}

void World::update_spawning(float tick_seconds) {
    // spawn de NPCs hostiles
    npc_spawn_timer += tick_seconds;
    if (npc_spawn_timer >= GameConfig::get_instance().get_npc_spawn_time_seconds()) {
        npc_spawn_timer = 0.0f;
        try_spawn_npc();
    }
}

std::vector<AttackResult> World::update(float tick_seconds) {
    update_players(tick_seconds);
    update_resurrections(tick_seconds);
    std::vector<AttackResult> attack_results = update_npcs(tick_seconds);
    update_spawning(tick_seconds);
    return attack_results;
}
