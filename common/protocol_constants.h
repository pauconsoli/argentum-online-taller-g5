#ifndef PROTOCOL_CONSTANTS_H
#define PROTOCOL_CONSTANTS_H

#include <cstdint>

namespace ClientOpcode {
constexpr uint8_t LOGIN = 0x01;
constexpr uint8_t LIST_MATCHES = 0x02;
constexpr uint8_t CREATE_MATCH = 0x03;
constexpr uint8_t JOIN_MATCH = 0x04;
constexpr uint8_t SELECT_RACE_CLASS = 0x05;

constexpr uint8_t MOVE = 0x10;
constexpr uint8_t ATTACK = 0x11;
constexpr uint8_t MEDITATE = 0x12;
constexpr uint8_t PICK_UP = 0x13;
constexpr uint8_t DROP_ITEM = 0x14;
constexpr uint8_t EQUIP_ITEM = 0x15;

constexpr uint8_t CHAT = 0x20;

constexpr uint8_t LEAVE_MATCH = 0xFE;
constexpr uint8_t DISCONNECT = 0xFF;
}  // namespace ClientOpcode

namespace ServerOpcode {
constexpr uint8_t LOGIN_OK = 0x81;
constexpr uint8_t MATCH_LIST = 0x82;
constexpr uint8_t MATCH_CREATED = 0x83;
constexpr uint8_t MATCH_JOINED = 0x84;
constexpr uint8_t SNAPSHOT = 0x85;
constexpr uint8_t PLAYER_JOINED = 0x86;
constexpr uint8_t PLAYER_LEFT = 0x87;
constexpr uint8_t PLAYER_SPAWNED = 0x88;  // jugador eligió raza/clase y se spawneó

constexpr uint8_t MOVED = 0x90;
constexpr uint8_t STATS = 0x91;
constexpr uint8_t DEATH = 0x92;
constexpr uint8_t REVIVE = 0x93;
constexpr uint8_t ATTACKED = 0x94;

constexpr uint8_t INVENTORY = 0xA0;

constexpr uint8_t CHAT_MSG = 0xB0;

constexpr uint8_t ERROR = 0xEE;
}  // namespace ServerOpcode

namespace ProtocolError {
constexpr uint8_t NICK_TAKEN = 0x01;
constexpr uint8_t MATCH_NOT_FOUND = 0x02;
constexpr uint8_t MATCH_FULL = 0x03;
constexpr uint8_t COMMAND_NOT_ALLOWED = 0x04;
constexpr uint8_t INVALID_ARG = 0x05;
}

#endif
