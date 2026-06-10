target_sources(argentum_tests
    PRIVATE
    # .cpp files - game module
    game/character_test.cpp
    game/player_test.cpp
    game/game_config_test.cpp
    game/game_formulas_test.cpp
    game/items_test.cpp
    game/inventory_test.cpp
    # .cpp files - world module
    world/cell_test.cpp
    world/zone_test.cpp
    world/world_map_test.cpp
    world/world_test.cpp
    world/attack_test.cpp
    PUBLIC
    # .h files
    )
