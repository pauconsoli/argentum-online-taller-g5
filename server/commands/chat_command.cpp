#include "common/commands/chat_command.h"

#include <string>
#include <utility>
#include <vector>

#include "common/updates/chat_msg_update.h"
#include "common/updates/system_msg_update.h"
#include "server/world/world.h"

std::vector<std::unique_ptr<GameUpdate>> ChatCommand::execute(World& world) {
    std::vector<std::unique_ptr<GameUpdate>> updates;

    if (!target_nick.empty()) {
        Player* recipient = world.get_player_by_name(target_nick);
        if (recipient == nullptr) {
            return updates;
        }

        uint32_t recipient_id = recipient->get_id();
        updates.push_back(
            std::make_unique<ChatMsgUpdate>(player_id, nick, text, recipient_id, true));
        if (recipient_id != player_id) {
            updates.push_back(
                std::make_unique<ChatMsgUpdate>(player_id, nick, text, player_id, true));
        }
        return updates;
    }

    updates.push_back(std::make_unique<ChatMsgUpdate>(player_id, nick, text));
    return updates;
}
