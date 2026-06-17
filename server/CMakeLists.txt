target_sources(argentum_server
    PRIVATE
    # .cpp files
    main.cpp
    server.cpp
    server_protocol.cpp
    acceptor_thread.cpp
    client_handler.cpp
    player_connection.cpp
    receiver_thread.cpp
    sender_thread.cpp
    gameloop_thread.cpp
    # commands
    commands/move_command.cpp
    commands/select_race_class_command.cpp
    commands/meditate_command.cpp
    commands/attack_command.cpp
    commands/drop_item_command.cpp
    commands/pick_up_item_command.cpp
    commands/equip_item_command.cpp
    commands/resurrect_command.cpp
    commands/interact_npc_command.cpp
    commands/chat_command.cpp
    # loaders
    loader/world_map_loader.cpp
    loader/game_config_loader.cpp
    loader/item_registry_loader.cpp
    loader/world_item_loader.cpp
    loader/npc_registry_loader.cpp
    PUBLIC
    # .h files
    server.h
    server_protocol.h
    server_ops.h
    acceptor_thread.h
    client_handler.h
    player_connection.h
    receiver_thread.h
    sender_thread.h
    gameloop_thread.h
    game/match.h
    loader/world_map_loader.h
    loader/game_config_loader.h
    loader/item_registry_loader.h
    loader/world_item_loader.h
    loader/npc_registry_loader.h
)