target_sources(taller_server
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
    # loaders
    loader/world_map_loader.cpp
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
    )
