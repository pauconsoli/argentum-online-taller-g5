#include "persister.h"

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <utility>

#include <nlohmann/json.hpp>

#include "world_serializer.h"

Persister::Persister(std::string save_path_): save_path(std::move(save_path_)) {}

std::optional<WorldSnapshot> Persister::try_load() {
    if (!std::filesystem::exists(save_path)) {
        std::cerr << "[PERSIST] No hay save previo en '" << save_path
                  << "'. Arranque limpio.\n";
        cache_loaded = true; 
        return std::nullopt;
    }

    try {
        std::ifstream in(save_path);
        if (!in.is_open()) {
            std::cerr << "[PERSIST] No se pudo abrir '" << save_path
                      << "' para lectura. Arranque limpio.\n";
            cache_loaded = true;
            return std::nullopt;
        }
        nlohmann::json j;
        in >> j;
        auto snapshot = WorldSerializer::from_json(j);
        std::cerr << "[PERSIST] Cargado save con " << snapshot.players.size()
                  << " jugador(es).\n";
        cached = snapshot;
        cache_loaded = true;
        return snapshot;
    } catch (const std::exception& e) {
        std::cerr << "[PERSIST] Save corrupto en '" << save_path << "': "
                  << e.what() << ". Arranque limpio.\n";
        cache_loaded = true;
        return std::nullopt;
    }
}

void Persister::save(const WorldSnapshot& snapshot) {
    WorldSnapshot to_write = snapshot;
    to_write.saved_at_unix = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    nlohmann::json j = WorldSerializer::to_json(to_write);
    std::string serialized = j.dump(2); 

    std::string tmp_path = save_path + ".tmp";
    {
        std::ofstream out(tmp_path, std::ios::out | std::ios::trunc);
        if (!out.is_open()) {
            throw std::runtime_error("Persister: no se pudo abrir '" + tmp_path +
                                     "' para escritura");
        }
        out << serialized;
        if (!out.good()) {
            throw std::runtime_error("Persister: error al escribir '" + tmp_path + "'");
        }
    }
    // rename atómico (POSIX rename(2))
    if (std::rename(tmp_path.c_str(), save_path.c_str()) != 0) {
        throw std::runtime_error("Persister: rename de '" + tmp_path + "' a '" +
                                 save_path + "' falló");
    }

    // 4) Refrescamos el cache para que find_player_save vea lo último.
    cached = to_write;
    cache_loaded = true;
}

std::optional<PlayerSave> Persister::find_player_save(const std::string& nick,
                                                       const std::string& match_name) {
    if (!cache_loaded) {
        try_load();
    }
    if (!cached.has_value()) {
        return std::nullopt;
    }
    for (const auto& p: cached->players) {
        if (p.nick == nick && p.match_name == match_name) {
            return p;
        }
    }
    return std::nullopt;
}
