target_sources(taller_client
    PRIVATE
    # .cpp files
    main.cpp
    sdl/game_client.cpp
    sdl/renderer.cpp
    PUBLIC
    # .h files
    sdl/game_client.h
    sdl/renderer.h
    )