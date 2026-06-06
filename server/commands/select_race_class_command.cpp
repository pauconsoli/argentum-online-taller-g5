#include "common/commands/select_race_class_command.h"

#include <stdexcept>

#include "common/protocol_constants.h"
#include "common/updates/error_update.h"
#include "common/updates/spawned_update.h"
#include "server/game/game_formulas.h"
#include "server/world/world.h"

std::unique_ptr<GameUpdate> SelectRaceClassCommand::execute(World& world) {
    if (world.player_exists(player_id)) {
        return std::make_unique<ErrorUpdate>(ProtocolError::COMMAND_NOT_ALLOWED,
                                             "El jugador ya está conectado en el mundo.");
    }

    try {
        Position spawn = world.get_spawn_position();
        auto player = GameFormulas::create_initial_player(player_id, nick, race, klass, spawn);
        world.add_player(std::move(player));

        return std::make_unique<SpawnedUpdate>(player_id, nick, race, klass, spawn);
    } catch (const std::exception& e) {
        return std::make_unique<ErrorUpdate>(ProtocolError::COMMAND_NOT_ALLOWED, e.what());
    }
}
