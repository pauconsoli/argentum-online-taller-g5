#include "common/commands/chat_command.h"

#include <utility>
#include <vector>

#include "common/updates/chat_msg_update.h"
#include "server/world/world.h"

std::vector<std::unique_ptr<GameUpdate>> ChatCommand::execute(World& /*world*/) {
    std::vector<std::unique_ptr<GameUpdate>> updates;
    updates.push_back(std::make_unique<ChatMsgUpdate>(player_id, nick, text));
    return updates;
}
