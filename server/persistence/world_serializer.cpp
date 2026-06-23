#include "world_serializer.h"

#include <utility>

namespace WorldSerializer {

nlohmann::json to_json(const PlayerSave& p) {
    nlohmann::json inv = nlohmann::json::array();
    for (const auto& slot: p.inventory) {
        inv.push_back({
            {"item_name", slot.item_name},
            {"quantity", slot.quantity},
            {"equipped", slot.is_equipped},
        });
    }

    return {
        {"nick", p.nick},
        {"match_name", p.match_name},
        {"race", p.race},
        {"class", p.klass},
        {"level", p.level},
        {"xp", p.xp},
        {"hp", p.hp},
        {"max_hp", p.max_hp},
        {"mp", p.mp},
        {"max_mp", p.max_mp},
        {"gold", p.gold},
        {"pos", {{"x", p.pos_x}, {"y", p.pos_y}}},
        {"inventory", std::move(inv)},
    };
}

PlayerSave player_from_json(const nlohmann::json& j) {
    PlayerSave p;
    p.nick = j.at("nick").get<std::string>();
    p.match_name = j.at("match_name").get<std::string>();
    p.race = j.at("race").get<uint8_t>();
    p.klass = j.at("class").get<uint8_t>();
    p.level = j.at("level").get<uint16_t>();
    p.xp = j.at("xp").get<uint64_t>();
    p.hp = j.at("hp").get<int32_t>();
    p.max_hp = j.at("max_hp").get<int32_t>();
    p.mp = j.at("mp").get<int32_t>();
    p.max_mp = j.at("max_mp").get<int32_t>();
    p.gold = j.at("gold").get<uint64_t>();
    p.pos_x = j.at("pos").at("x").get<int32_t>();
    p.pos_y = j.at("pos").at("y").get<int32_t>();

    for (const auto& slot_json: j.at("inventory")) {
        InventorySlotSave slot;
        slot.item_name = slot_json.at("item_name").get<std::string>();
        slot.quantity = slot_json.at("quantity").get<uint32_t>();
        slot.is_equipped = slot_json.at("equipped").get<bool>();
        p.inventory.push_back(std::move(slot));
    }
    return p;
}

nlohmann::json to_json(const WorldSnapshot& s) {
    nlohmann::json players = nlohmann::json::array();
    for (const auto& p: s.players) {
        players.push_back(to_json(p));
    }
    nlohmann::json matches = nlohmann::json::array();
    for (const auto& m: s.matches) {
        matches.push_back({
            {"name", m.name},
            {"max_players", m.max_players},
        });
    }
    return {
        {"version", s.version},
        {"saved_at_unix", s.saved_at_unix},
        {"players", std::move(players)},
        {"matches", std::move(matches)},
    };
}

WorldSnapshot from_json(const nlohmann::json& j) {
    WorldSnapshot s;
    s.version = j.value("version", uint32_t{1});
    s.saved_at_unix = j.value("saved_at_unix", int64_t{0});
    for (const auto& player_json: j.at("players")) {
        s.players.push_back(player_from_json(player_json));
    }
    if (j.contains("matches")) {
        for (const auto& match_json: j.at("matches")) {
            MatchSave m;
            m.name = match_json.at("name").get<std::string>();
            m.max_players = match_json.at("max_players").get<uint8_t>();
            s.matches.push_back(std::move(m));
        }
    }
    return s;
}

}
