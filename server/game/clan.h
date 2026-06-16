#ifndef CLAN_H
#define CLAN_H

#include <cstdint>
#include <set>
#include <string>
#include <vector>

class Clan {
 private:
    uint32_t clan_id;
    std::string name;
    uint32_t founder_id;

    std::set<uint32_t> members;  // incluye al fundador
    std::set<uint32_t> pending;
    std::set<uint32_t> banned;

 public:
    Clan(uint32_t clan_id, const std::string& name, uint32_t founder_id);

    uint32_t get_id() const;
    const std::string& get_name() const;
    uint32_t get_founder_id() const;

    bool request_join(uint32_t player_id);
    bool accept(uint32_t player_id);
    bool reject(uint32_t player_id);
    void ban(uint32_t player_id);
    bool kick(uint32_t player_id);
    bool leave(uint32_t player_id);

    bool is_member(uint32_t player_id) const;
    bool is_pending(uint32_t player_id) const;
    bool is_banned(uint32_t player_id) const;
    bool is_full() const;

    const std::set<uint32_t>& get_members() const;
    const std::set<uint32_t>& get_pending() const;

    int member_count() const;
};

#endif
