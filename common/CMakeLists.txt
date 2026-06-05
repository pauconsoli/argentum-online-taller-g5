target_sources(taller_common
    PRIVATE
    # .cpp files
    liberror.cpp
    resolver.cpp
    resolvererror.cpp
    socket.cpp
    PUBLIC
    # .h files
    direction.h
    liberror.h
    position.h
    protocol_constants.h
    queue.h
    resolver.h
    resolvererror.h
    socket.h
    thread.h
    # commands
    commands/client_command.h
    commands/move_command.h
    commands/select_race_class_command.h
    commands/meditate_command.h
    # updates
    updates/error_update.h
    updates/game_update.h
    updates/login_ok_update.h
    updates/match_created_update.h
    updates/match_joined_update.h
    updates/match_list_update.h
    updates/moved_update.h
    updates/player_joined_update.h
    updates/player_left_update.h
    updates/snapshot_update.h
    updates/spawned_update.h
    )
