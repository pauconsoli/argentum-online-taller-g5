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
    attack_result.h
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
    commands/attack_command.h
    commands/equip_item_command.h
    commands/drop_item_command.h
    commands/pick_up_item_command.h
    commands/resurrect_command.h
    commands/interact_npc_command.h
    # items
    commands/chat_command.h
    # world
    world/terrain_type.h
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
    updates/attack_update.h
    updates/death_update.h
    updates/chat_msg_update.h
    )
