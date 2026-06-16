#include "clan.h"

#include "server/game/game_config.h"

Clan::Clan(uint32_t clan_id, const std::string& name, uint32_t founder_id):
    clan_id(clan_id), name(name), founder_id(founder_id) {
    members.insert(founder_id);
}

uint32_t Clan::get_id() const {
    return clan_id;
}

const std::string& Clan::get_name() const {
    return name;
}

uint32_t Clan::get_founder_id() const {
    return founder_id;
}

bool Clan::request_join(uint32_t player_id) {
    if (is_member(player_id) || is_pending(player_id) || is_banned(player_id) || is_full()) {
        return false;
    }
    pending.insert(player_id);
    return true;
}

bool Clan::accept(uint32_t player_id) {
    if (!is_pending(player_id))
        return false;
    if (is_full())
        return false;
    pending.erase(player_id);
    members.insert(player_id);
    return true;
}

bool Clan::reject(uint32_t player_id) {
    if (!is_pending(player_id))
        return false;
    pending.erase(player_id);
    return true;
}

void Clan::ban(uint32_t player_id) {
    pending.erase(player_id);  // cancela el pedido si lo tenía
    members.erase(player_id);  // expulsa si era miembro
    banned.insert(player_id);
}

bool Clan::kick(uint32_t player_id) {
    if (player_id == founder_id)
        return false;
    if (!is_member(player_id))
        return false;
    members.erase(player_id);
    return true;
}

bool Clan::leave(uint32_t player_id) {
    if (player_id == founder_id)
        return false;  // el fundador no puede irse
    if (!is_member(player_id))
        return false;
    members.erase(player_id);
    return true;
}

bool Clan::is_member(uint32_t player_id) const {
    return members.count(player_id) > 0;
}
bool Clan::is_pending(uint32_t player_id) const {
    return pending.count(player_id) > 0;
}
bool Clan::is_banned(uint32_t player_id) const {
    return banned.count(player_id) > 0;
}
bool Clan::is_full() const {
    return static_cast<int>(members.size()) >= GameConfig::get_instance().get_clan_max_members();
}

const std::set<uint32_t>& Clan::get_members() const {
    return members;
}
const std::set<uint32_t>& Clan::get_pending() const {
    return pending;
}
int Clan::member_count() const {
    return static_cast<int>(members.size());
}
