#include <sys/socket.h>
#include <unistd.h>

#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "client/client_protocol.h"
#include "common/protocol_constants.h"
#include "common/socket.h"
#include "common/updates/attack_update.h"
#include "common/updates/chat_msg_update.h"
#include "common/updates/death_update.h"
#include "common/updates/error_update.h"
#include "common/updates/inventory_update.h"
#include "common/updates/login_ok_update.h"
#include "common/updates/match_created_update.h"
#include "common/updates/match_joined_update.h"
#include "common/updates/match_list_update.h"
#include "common/updates/moved_update.h"
#include "common/updates/player_joined_update.h"
#include "common/updates/player_left_update.h"
#include "common/updates/world_map_update.h"
#include "server/server_protocol.h"

namespace {

class ProtocolFixture: public ::testing::Test {
 protected:
    int client_fd = -1;
    int server_fd = -1;
    std::unique_ptr<Socket> client_socket;
    std::unique_ptr<Socket> server_socket;
    std::unique_ptr<ClientProtocol> client_protocol;
    std::unique_ptr<ServerProtocol> server_protocol;

    void SetUp() override {
        int fds[2];
        ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0)
            << "No se pudo crear socketpair";
        client_fd = fds[0];
        server_fd = fds[1];

        client_socket = std::make_unique<Socket>(client_fd);
        server_socket = std::make_unique<Socket>(server_fd);
        client_protocol = std::make_unique<ClientProtocol>(*client_socket);
        server_protocol = std::make_unique<ServerProtocol>(*server_socket);
    }

};

TEST_F(ProtocolFixture, LoginRoundTrip) {
    client_protocol->send_login("reni");

    EXPECT_EQ(server_protocol->recv_opcode(), ClientOpcode::LOGIN);
    EXPECT_EQ(server_protocol->recv_login_payload(), "reni");
}

TEST_F(ProtocolFixture, LoginConTildesYCaracteresUTF8) {
    client_protocol->send_login("ñiño_ágil");

    EXPECT_EQ(server_protocol->recv_opcode(), ClientOpcode::LOGIN);
    EXPECT_EQ(server_protocol->recv_login_payload(), "ñiño_ágil");
}

TEST_F(ProtocolFixture, ListMatchesNoLlevaPayload) {
    client_protocol->send_list_matches();
    EXPECT_EQ(server_protocol->recv_opcode(), ClientOpcode::LIST_MATCHES);
}

TEST_F(ProtocolFixture, CreateMatchRoundTrip) {
    client_protocol->send_create_match("torneo-viernes", 6);

    EXPECT_EQ(server_protocol->recv_opcode(), ClientOpcode::CREATE_MATCH);
    auto payload = server_protocol->recv_create_match_payload();
    EXPECT_EQ(payload.name, "torneo-viernes");
    EXPECT_EQ(payload.max_players, 6);
}

TEST_F(ProtocolFixture, JoinMatchRoundTrip) {
    client_protocol->send_join_match(42);

    EXPECT_EQ(server_protocol->recv_opcode(), ClientOpcode::JOIN_MATCH);
    EXPECT_EQ(server_protocol->recv_join_match_payload(), 42u);
}

TEST_F(ProtocolFixture, JoinMatchPreservaIdsGrandes) {
    const uint32_t big_id = 0xFFFF0001u;
    client_protocol->send_join_match(big_id);

    EXPECT_EQ(server_protocol->recv_opcode(), ClientOpcode::JOIN_MATCH);
    EXPECT_EQ(server_protocol->recv_join_match_payload(), big_id);
}

TEST_F(ProtocolFixture, ChatRoundTrip) {
    client_protocol->send_chat("hola gente");

    EXPECT_EQ(server_protocol->recv_opcode(), ClientOpcode::CHAT);
}

TEST_F(ProtocolFixture, ChatStringVacioFunciona) {
    client_protocol->send_chat("");
    EXPECT_EQ(server_protocol->recv_opcode(), ClientOpcode::CHAT);
}

TEST_F(ProtocolFixture, DisconnectEsSoloOpcode) {
    client_protocol->send_disconnect();
    EXPECT_EQ(server_protocol->recv_opcode(), ClientOpcode::DISCONNECT);
}

TEST_F(ProtocolFixture, LeaveMatchEsSoloOpcode) {
    client_protocol->send_leave_match();
    EXPECT_EQ(server_protocol->recv_opcode(), ClientOpcode::LEAVE_MATCH);
}

TEST_F(ProtocolFixture, LoginOkRoundTrip) {
    LoginOkUpdate update(12345);
    server_protocol->send_update(update);

    auto received = client_protocol->receive_update();
    ASSERT_NE(received, nullptr);
    ASSERT_EQ(received->get_type(), UpdateType::LOGIN_OK);
    auto* login_ok = dynamic_cast<LoginOkUpdate*>(received.get());
    ASSERT_NE(login_ok, nullptr);
    EXPECT_EQ(login_ok->player_id, 12345u);
}

TEST_F(ProtocolFixture, MatchListVacioRoundTrip) {
    MatchListUpdate update({});
    server_protocol->send_update(update);

    auto received = client_protocol->receive_update();
    ASSERT_EQ(received->get_type(), UpdateType::MATCH_LIST);
    auto* match_list = dynamic_cast<MatchListUpdate*>(received.get());
    ASSERT_NE(match_list, nullptr);
    EXPECT_EQ(match_list->matches.size(), 0u);
}

TEST_F(ProtocolFixture, MatchListConVariasPartidasRoundTrip) {
    std::vector<MatchInfo> matches = {
        {1, "torneo", 3, 8},
        {2, "casual", 1, 4},
        {3, "duelo", 2, 2},
    };
    MatchListUpdate update(std::move(matches));
    server_protocol->send_update(update);

    auto received = client_protocol->receive_update();
    auto* match_list = dynamic_cast<MatchListUpdate*>(received.get());
    ASSERT_NE(match_list, nullptr);
    ASSERT_EQ(match_list->matches.size(), 3u);
    EXPECT_EQ(match_list->matches[0].id, 1u);
    EXPECT_EQ(match_list->matches[0].name, "torneo");
    EXPECT_EQ(match_list->matches[0].current_players, 3);
    EXPECT_EQ(match_list->matches[0].max_players, 8);
    EXPECT_EQ(match_list->matches[2].name, "duelo");
}

TEST_F(ProtocolFixture, MatchCreatedRoundTrip) {
    MatchCreatedUpdate update(99);
    server_protocol->send_update(update);

    auto received = client_protocol->receive_update();
    auto* match_created = dynamic_cast<MatchCreatedUpdate*>(received.get());
    ASSERT_NE(match_created, nullptr);
    EXPECT_EQ(match_created->match_id, 99u);
}

TEST_F(ProtocolFixture, MatchJoinedSinRestoreRoundTrip) {
    MatchJoinedUpdate update(7, 42);
    server_protocol->send_update(update);

    auto received = client_protocol->receive_update();
    auto* mj = dynamic_cast<MatchJoinedUpdate*>(received.get());
    ASSERT_NE(mj, nullptr);
    EXPECT_EQ(mj->match_id, 7u);
    EXPECT_EQ(mj->your_player_id, 42u);
    EXPECT_FALSE(mj->was_restored);
    EXPECT_EQ(mj->restored_race, 0);
    EXPECT_EQ(mj->restored_klass, 0);
}

TEST_F(ProtocolFixture, MatchJoinedConRestoreRoundTrip) {
    MatchJoinedUpdate update(7, 42, /*was_restored=*/true, /*race=*/2, /*klass=*/3);
    server_protocol->send_update(update);

    auto received = client_protocol->receive_update();
    auto* mj = dynamic_cast<MatchJoinedUpdate*>(received.get());
    ASSERT_NE(mj, nullptr);
    EXPECT_TRUE(mj->was_restored);
    EXPECT_EQ(mj->restored_race, 2);
    EXPECT_EQ(mj->restored_klass, 3);
}

TEST_F(ProtocolFixture, AttackResultConArmaYTipoRoundTrip) {
    AttackResult result;
    result.attacker_id = 1;
    result.target_id = 2;
    result.damage = 25;
    result.evaded = false;
    result.target_died = false;
    result.is_healing = false;
    result.heal_amount = 0;
    result.type = AttackType::RANGED;
    result.weapon_or_spell_name = "Arco simple";

    AttackUpdate update(result);
    server_protocol->send_update(update);

    auto received = client_protocol->receive_update();
    auto* attack = dynamic_cast<AttackUpdate*>(received.get());
    ASSERT_NE(attack, nullptr);
    const AttackResult& r = attack->get_result();
    EXPECT_EQ(r.attacker_id, 1u);
    EXPECT_EQ(r.target_id, 2u);
    EXPECT_EQ(r.damage, 25);
    EXPECT_FALSE(r.evaded);
    EXPECT_EQ(r.type, AttackType::RANGED);
    EXPECT_EQ(r.weapon_or_spell_name, "Arco simple");
}

TEST_F(ProtocolFixture, AttackResultConHealingRoundTrip) {
    AttackResult result;
    result.attacker_id = 5;
    result.target_id = 5; 
    result.damage = 0;
    result.is_healing = true;
    result.heal_amount = 30;
    result.type = AttackType::MAGIC;
    result.weapon_or_spell_name = "Curar";

    AttackUpdate update(result);
    server_protocol->send_update(update);

    auto received = client_protocol->receive_update();
    auto* attack = dynamic_cast<AttackUpdate*>(received.get());
    ASSERT_NE(attack, nullptr);
    EXPECT_TRUE(attack->get_result().is_healing);
    EXPECT_EQ(attack->get_result().heal_amount, 30);
    EXPECT_EQ(attack->get_result().type, AttackType::MAGIC);
    EXPECT_EQ(attack->get_result().weapon_or_spell_name, "Curar");
}

TEST_F(ProtocolFixture, DeathUpdateRoundTrip) {
    DeathUpdate update(42, 7);
    server_protocol->send_update(update);

    auto received = client_protocol->receive_update();
    auto* death = dynamic_cast<DeathUpdate*>(received.get());
    ASSERT_NE(death, nullptr);
    EXPECT_EQ(death->get_dead_id(), 42u);
    EXPECT_EQ(death->get_killer_id(), 7u);
}

TEST_F(ProtocolFixture, MovedRoundTrip) {
    MovedUpdate update(42, Position{15, 23});
    server_protocol->send_update(update);

    auto received = client_protocol->receive_update();
    auto* moved = dynamic_cast<MovedUpdate*>(received.get());
    ASSERT_NE(moved, nullptr);
    EXPECT_EQ(moved->get_player_id(), 42u);
    EXPECT_EQ(moved->get_pos().x, 15);
    EXPECT_EQ(moved->get_pos().y, 23);
}

TEST_F(ProtocolFixture, MovedConCoordenadasNegativasRoundTrip) {
    MovedUpdate update(42, Position{-50, -100});
    server_protocol->send_update(update);

    auto received = client_protocol->receive_update();
    auto* moved = dynamic_cast<MovedUpdate*>(received.get());
    ASSERT_NE(moved, nullptr);
    EXPECT_EQ(moved->get_pos().x, -50);
    EXPECT_EQ(moved->get_pos().y, -100);
}

TEST_F(ProtocolFixture, InventoryConItemsRoundTrip) {
    std::vector<InventorySlotData> items = {
        {"espada", 1, true},
        {"pocion_vida", 5, false},
        {"", 0, false},  // slot vacío
    };
    InventoryUpdate update(99, std::move(items), /*gold=*/1500);
    server_protocol->send_update(update);

    auto received = client_protocol->receive_update();
    auto* inv = dynamic_cast<InventoryUpdate*>(received.get());
    ASSERT_NE(inv, nullptr);
    EXPECT_EQ(inv->get_target_player_id(), 99u);
    EXPECT_EQ(inv->get_gold(), 1500u);
    ASSERT_EQ(inv->get_items().size(), 3u);
    EXPECT_EQ(inv->get_items()[0].item_name, "espada");
    EXPECT_TRUE(inv->get_items()[0].is_equipped);
    EXPECT_EQ(inv->get_items()[1].quantity, 5u);
}

TEST_F(ProtocolFixture, InventoryConGoldEnHi32RoundTrip) {
    // El protocolo serializa uint64 como dos uint32 (hi, lo). Verificamos que
    // un valor con bits altos sobreviva.
    const uint64_t big_gold = (uint64_t{0x12345678} << 32) | 0x9ABCDEF0;
    InventoryUpdate update(1, {}, big_gold);
    server_protocol->send_update(update);

    auto received = client_protocol->receive_update();
    auto* inv = dynamic_cast<InventoryUpdate*>(received.get());
    ASSERT_NE(inv, nullptr);
    EXPECT_EQ(inv->get_gold(), big_gold);
}

TEST_F(ProtocolFixture, ChatMsgBroadcastRoundTrip) {
    ChatMsgUpdate update(7, "reni", "hola gente");
    server_protocol->send_update(update);

    auto received = client_protocol->receive_update();
    auto* chat = dynamic_cast<ChatMsgUpdate*>(received.get());
    ASSERT_NE(chat, nullptr);
    EXPECT_EQ(chat->sender_id, 7u);
    EXPECT_EQ(chat->sender_nick, "reni");
    EXPECT_EQ(chat->text, "hola gente");
}

TEST_F(ProtocolFixture, WorldMapPequenoRoundTrip) {
    std::vector<MapCellData> cells = {
        {0, false}, {0, false}, {1, true}, {0, false},
        {0, false}, {1, true},  {1, true}, {0, false},
    };
    WorldMapUpdate update(4, 2, cells);
    server_protocol->send_update(update);

    auto received = client_protocol->receive_update();
    auto* world_map = dynamic_cast<WorldMapUpdate*>(received.get());
    ASSERT_NE(world_map, nullptr);
    EXPECT_EQ(world_map->width, 4);
    EXPECT_EQ(world_map->height, 2);
    ASSERT_EQ(world_map->cells.size(), 8u);
    EXPECT_EQ(world_map->cells[2].terrain_type, 1);
    EXPECT_TRUE(world_map->cells[2].blocking);
    EXPECT_FALSE(world_map->cells[3].blocking);
}

TEST_F(ProtocolFixture, ErrorUpdateRoundTrip) {
    ErrorUpdate update(42, ProtocolError::NICK_TAKEN, "El nick 'reni' ya está en uso");
    server_protocol->send_update(update);

    auto received = client_protocol->receive_update();
    auto* err = dynamic_cast<ErrorUpdate*>(received.get());
    ASSERT_NE(err, nullptr);
    EXPECT_EQ(err->code, ProtocolError::NICK_TAKEN);
    EXPECT_EQ(err->detail, "El nick 'reni' ya está en uso");
}

TEST_F(ProtocolFixture, PlayerJoinedRoundTrip) {
    PlayerJoinedUpdate update(7, "chiari", /*race=*/1, /*klass=*/3, Position{10, 20});
    server_protocol->send_update(update);

    auto received = client_protocol->receive_update();
    auto* pj = dynamic_cast<PlayerJoinedUpdate*>(received.get());
    ASSERT_NE(pj, nullptr);
    EXPECT_EQ(pj->player_id, 7u);
    EXPECT_EQ(pj->nick, "chiari");
    EXPECT_EQ(pj->race, 1);
    EXPECT_EQ(pj->klass, 3);
    EXPECT_EQ(pj->pos.x, 10);
    EXPECT_EQ(pj->pos.y, 20);
}

TEST_F(ProtocolFixture, PlayerLeftRoundTrip) {
    PlayerLeftUpdate update(7, "reni", /*clan_id=*/0);
    server_protocol->send_update(update);

    auto received = client_protocol->receive_update();
    auto* pl = dynamic_cast<PlayerLeftUpdate*>(received.get());
    ASSERT_NE(pl, nullptr);
    EXPECT_EQ(pl->player_id, 7u);
}

// ---------- Comportamiento ante errores ----------

TEST_F(ProtocolFixture, OpcodesSecuencialesNoSeMezclan) {
    client_protocol->send_list_matches();
    client_protocol->send_disconnect();

    EXPECT_EQ(server_protocol->recv_opcode(), ClientOpcode::LIST_MATCHES);
    EXPECT_EQ(server_protocol->recv_opcode(), ClientOpcode::DISCONNECT);
}

TEST_F(ProtocolFixture, VariasUpdatesEnFilaPreservanOrden) {
    LoginOkUpdate login(1);
    MatchCreatedUpdate created(42);
    PlayerLeftUpdate left(7, "chiari", /*clan_id=*/0);

    server_protocol->send_update(login);
    server_protocol->send_update(created);
    server_protocol->send_update(left);

    auto u1 = client_protocol->receive_update();
    auto u2 = client_protocol->receive_update();
    auto u3 = client_protocol->receive_update();

    EXPECT_EQ(u1->get_type(), UpdateType::LOGIN_OK);
    EXPECT_EQ(u2->get_type(), UpdateType::MATCH_CREATED);
    EXPECT_EQ(u3->get_type(), UpdateType::PLAYER_LEFT);
}

}
