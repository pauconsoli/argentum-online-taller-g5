#include "common/commands/chat_command.h"

#include <string>
#include <utility>
#include <vector>

#include "common/updates/chat_msg_update.h"
#include "common/updates/system_msg_update.h"
#include "server/world/world.h"

std::vector<std::unique_ptr<GameUpdate>> ChatCommand::execute(World& world) {
    std::vector<std::unique_ptr<GameUpdate>> updates;

    if (!text.empty() && text[0] == '@') {
        size_t space_pos = text.find(' ');
        if (space_pos != std::string::npos && space_pos > 1) {
            std::string target_nick = text.substr(1, space_pos - 1);
            std::string private_text = text.substr(space_pos + 1);

            Player* recipient = world.get_player_by_name(target_nick);
            if (recipient == nullptr) {
                updates.push_back(std::make_unique<SystemMsgUpdate>(
                    player_id, "El jugador '" + target_nick + "' no está conectado."));
                return updates;
            }

            uint32_t recipient_id = recipient->get_id();
            updates.push_back(
                std::make_unique<ChatMsgUpdate>(player_id, nick, private_text, recipient_id, true));
            if (recipient_id != player_id) {
                updates.push_back(std::make_unique<ChatMsgUpdate>(player_id, nick, private_text,
                                                                  player_id, true));
            }
            return updates;
        }
    }

    updates.push_back(std::make_unique<ChatMsgUpdate>(player_id, nick, text));
    return updates;
}
