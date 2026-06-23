target_sources(argentum_tests
    PRIVATE
    # .cpp files - game module
    game/character_test.cpp
    game/player_test.cpp
    game/game_config_test.cpp
    game/game_formulas_test.cpp
    game/items_test.cpp
    game/inventory_test.cpp
    game/npc_test.cpp

    # .cpp files - world module
    world/cell_test.cpp
    world/zone_test.cpp
    world/world_map_test.cpp
    world/world_test.cpp
    world/attack_test.cpp

    # .cpp files - protocol module
    protocol/protocol_roundtrip_test.cpp
    # Fuentes del protocolo que necesitan estar en el binario de tests.
    ${CMAKE_SOURCE_DIR}/client/client_protocol.cpp
    ${CMAKE_SOURCE_DIR}/server/server_protocol.cpp
    ${CMAKE_SOURCE_DIR}/server/commands/move_command.cpp
    ${CMAKE_SOURCE_DIR}/server/commands/attack_command.cpp
    ${CMAKE_SOURCE_DIR}/server/commands/meditate_command.cpp
    ${CMAKE_SOURCE_DIR}/server/commands/pick_up_item_command.cpp
    ${CMAKE_SOURCE_DIR}/server/commands/drop_item_command.cpp
    ${CMAKE_SOURCE_DIR}/server/commands/equip_item_command.cpp
    ${CMAKE_SOURCE_DIR}/server/commands/chat_command.cpp
    ${CMAKE_SOURCE_DIR}/server/commands/interact_npc_command.cpp
    ${CMAKE_SOURCE_DIR}/server/commands/clan_command.cpp
    ${CMAKE_SOURCE_DIR}/server/commands/cheat_command.cpp
    ${CMAKE_SOURCE_DIR}/server/commands/resurrect_command.cpp
    PUBLIC
    # .h files
    )
