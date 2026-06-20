cmake_minimum_required(VERSION 3.24)

if (POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif()

set(CMAKE_CXX_EXTENSIONS ON)

project(taller_tp
    VERSION 1.0
    HOMEPAGE_URL "https://github.com/URL-HERE"
    LANGUAGES CXX
    )

option(ARGENTUM_TESTS "Enable / disable tests." ON)
option(ARGENTUM_CLIENT "Enable / disable client program." ON)
option(ARGENTUM_CLIENT_CLI "Enable / disable client CLI (debug, sin GUI)." ON)
option(ARGENTUM_CLIENT_QT "Enable / disable client con menú Qt." ON)
option(ARGENTUM_SERVER "Enable / disable server program." ON)
option(TALLER_MAKE_WARNINGS_AS_ERRORS "Enable / disable warnings as errors." ON)
option(TALLER_ENABLE_ASAN "Enable AddressSanitizer (ASAN) for memory error detection." OFF)
option(TALLER_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer (UBSAN)." OFF)

message(CMAKE_CXX_COMPILER_ID="${CMAKE_CXX_COMPILER_ID}")

# Configure sanitizers
# NOTE: We do NOT use add_compile_options/add_link_options here because that
# would apply ASAN/UBSAN to all FetchContent dependencies (SDL2, etc.) causing
# undefined references to ASAN runtime symbols when linking shared libraries.
# Instead, we collect the flags and apply them per-target using a helper macro.
set(TALLER_SANITIZER_FLAGS "")
if(TALLER_ENABLE_ASAN)
    message(STATUS "ASAN enabled")
    list(APPEND TALLER_SANITIZER_FLAGS -fsanitize=address)
endif()

if(TALLER_ENABLE_UBSAN)
    message(STATUS "UBSAN enabled")
    list(APPEND TALLER_SANITIZER_FLAGS -fsanitize=undefined)
endif()

macro(taller_apply_sanitizers target)
    if(TALLER_SANITIZER_FLAGS)
        target_compile_options(${target} PRIVATE ${TALLER_SANITIZER_FLAGS})
        target_link_options(${target} PRIVATE ${TALLER_SANITIZER_FLAGS})
    endif()
endmacro()

# Fetch spdlog for logging
include(FetchContent)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.14.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(spdlog)

# Fetch toml++ for TOML config file parsing
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(tomlplusplus)

# Define target: lib common between client and server programs
# HEY TODO XXX: change the name to something that you like
add_library(taller_common STATIC)

# add the source files that make the lib
# HEY!! TODO XXX: add more subfolders here
add_subdirectory(common/)

# Define target: lib game (server-side game logic)
add_library(taller_game STATIC)
add_subdirectory(server/game)

# Define target: lib world (server-side world/map logic)
add_library(taller_world STATIC)
add_subdirectory(server/world)

# Linker dependencies between the libs
target_link_libraries(taller_world PUBLIC taller_game)
target_link_libraries(taller_game PUBLIC taller_world)

# set compiler warnings to the targets
include(cmake/CompilerWarnings.cmake)
set_project_warnings(taller_common ${TALLER_MAKE_WARNINGS_AS_ERRORS} FALSE)
set_project_warnings(taller_game ${TALLER_MAKE_WARNINGS_AS_ERRORS} FALSE)
set_project_warnings(taller_world ${TALLER_MAKE_WARNINGS_AS_ERRORS} FALSE)
taller_apply_sanitizers(taller_common)
taller_apply_sanitizers(taller_game)
taller_apply_sanitizers(taller_world)

# Disable -Werror=format-nonliteral for taller_common because of warning in liberror.cpp
target_compile_options(taller_common PRIVATE -Wno-error=format-nonliteral)

# Say where the headers will be found required by library target
# Make this PUBLIC so any dependant linking lib will inherit this
# location (so our dependants will not have to explicitly request this)
target_include_directories(taller_common PUBLIC .)
target_include_directories(taller_game PUBLIC .)
target_include_directories(taller_world PUBLIC .)

# HEY!! TODO XXX: you need to install some runtime and dev libraries *before*
# compiling the client code.
#
# In xubuntu 24.04 these are:
#
# sudo apt-get install libopus-dev libopusfile-dev libxmp-dev libfluidsynth-dev fluidsynth libwavpack1 libwavpack-dev libfreetype-dev wavpack
#

# Install libSDL2 and libSDL2pp (for client)
# HEY!! TODO XXX: add more dependendies (either for client or server)
if(ARGENTUM_CLIENT OR ARGENTUM_CLIENT_QT)
    include(FetchContent)
    FetchContent_Declare(
        SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-2.30.8
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        OVERRIDE_FIND_PACKAGE
    )

    FetchContent_Declare(
        SDL2_image
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
        GIT_TAG release-2.8.2
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        OVERRIDE_FIND_PACKAGE
    )

    FetchContent_Declare(
        SDL2_mixer
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
        GIT_TAG release-2.8.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        OVERRIDE_FIND_PACKAGE
    )

    FetchContent_Declare(
        SDL2_ttf
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
        GIT_TAG release-2.22.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        OVERRIDE_FIND_PACKAGE
    )

    FetchContent_Declare(
        libSDL2pp
        OVERRIDE_FIND_PACKAGE
        URL https://github.com/libSDL2pp/libSDL2pp/archive/cc198c9a5657048bee67ece82de620b2d5661084.zip
    )
    # Disable Opus support in SDL2_mixer (broken on macOS with Homebrew opusfile)
    set(SDL2MIXER_OPUS OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(SDL2 SDL2_image SDL2_mixer SDL2_ttf libSDL2pp)
endif()

# Program section
# ---------------
if(ARGENTUM_CLIENT)
    # HEY!! TODO XXX: Change the name of the executable to the one you like!
    add_executable(argentum_client)

    # Make them depend on taller_common lib
    add_dependencies(argentum_client taller_common SDL2pp::SDL2pp)

    # Add source files and enable warnings
    # HEY!! TODO XXX: add more subfolders here
    add_subdirectory(client)

    set_project_warnings(argentum_client ${TALLER_MAKE_WARNINGS_AS_ERRORS} FALSE)
    taller_apply_sanitizers(argentum_client)

    # add libSDL2pp's headers
    target_include_directories(argentum_client PRIVATE ${libSDL2pp_SOURCE_DIR})
    target_include_directories(argentum_client PRIVATE ${sdl2_image_SOURCE_DIR}/include)
    target_include_directories(argentum_client PRIVATE ${sdl2_ttf_SOURCE_DIR})
    target_include_directories(argentum_client PRIVATE ${sdl2_mixer_SOURCE_DIR}/include)


    # Link the taller_common lib target
    target_link_libraries(argentum_client taller_common SDL2pp::SDL2pp SDL2_image SDL2_ttf SDL2_mixer spdlog::spdlog tomlplusplus::tomlplusplus)
endif()

# Binario CLI del cliente (sin GUI). Para smoke-testear el server mientras
# Qt/SDL no estén listos. Comparte client.cpp y client_protocol.cpp con
# argentum_client pero usa un main propio.
if(ARGENTUM_CLIENT_CLI)
    add_executable(argentum_client_cli
        client/client_main.cpp
        client/client.cpp
        client/client_protocol.cpp
    )
    add_dependencies(argentum_client_cli taller_common)
    set_project_warnings(argentum_client_cli ${TALLER_MAKE_WARNINGS_AS_ERRORS} FALSE)
    taller_apply_sanitizers(argentum_client_cli)
    target_link_libraries(argentum_client_cli taller_common spdlog::spdlog)
endif()

if(ARGENTUM_CLIENT_QT)
    find_package(Qt5 COMPONENTS Core Widgets QUIET)
    if(Qt5_FOUND)
        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTORCC ON)

        add_executable(argentum_client_qt
            client/qt/qt_main.cpp
            client/qt/main_window.cpp
            client/qt/main_window.h
            client/qt/qt_client_adapter.cpp
            client/qt/qt_client_adapter.h
            client/qt/connection_widget.cpp
            client/qt/connection_widget.h
            client/qt/lobby_widget.cpp
            client/qt/lobby_widget.h
            client/qt/race_class_widget.cpp
            client/qt/race_class_widget.h
            client/client.cpp
            client/client.h
            client/client_protocol.cpp
            client/sdl/game_client.cpp
            client/sdl/sdl_config.cpp
            client/sdl/renderer.cpp
            client/sdl/character_renderer.cpp
            client/sdl/input_handler.cpp
            client/sdl/camera.cpp
            client/sdl/hud.cpp
            client/sdl/mini_chat.cpp
            client/sdl/sprite_manager.cpp
            client/sdl/client_map.cpp
            client/sdl/audio_manager.cpp
            client/sdl/terrain_renderer.cpp
        )

        add_dependencies(argentum_client_qt taller_common SDL2pp::SDL2pp)

        set_project_warnings(argentum_client_qt ${TALLER_MAKE_WARNINGS_AS_ERRORS} FALSE)
        taller_apply_sanitizers(argentum_client_qt)

        target_include_directories(argentum_client_qt PRIVATE
            ${libSDL2pp_SOURCE_DIR}
            ${sdl2_image_SOURCE_DIR}/include
            ${sdl2_ttf_SOURCE_DIR}
            ${sdl2_mixer_SOURCE_DIR}/include
        )

        target_link_libraries(argentum_client_qt
            taller_common
            Qt5::Core
            Qt5::Widgets
            SDL2pp::SDL2pp
            SDL2_image
            SDL2_ttf
            SDL2_mixer
            spdlog::spdlog
            tomlplusplus::tomlplusplus
        )
    else()
        message(WARNING "Qt5 no encontrado. Saltando argentum_client_qt. Instalalo con: sudo apt-get install qtbase5-dev")
    endif()
endif()

if(ARGENTUM_SERVER)
    # HEY!! TODO XXX: Change the name of the executable to the one you like!
    add_executable(argentum_server)

    # Make them depend on taller_common, taller_game and taller_world libs
    add_dependencies(argentum_server taller_common taller_game taller_world)

    # Add source files and enable warnings
    # HEY!! TODO XXX: add more subfolders here
    add_subdirectory(server)

    set_project_warnings(argentum_server ${TALLER_MAKE_WARNINGS_AS_ERRORS} FALSE)
    taller_apply_sanitizers(argentum_server)

    # Link the taller_common, taller_game and taller_world lib targets
    target_link_libraries(argentum_server taller_common taller_game taller_world spdlog::spdlog tomlplusplus::tomlplusplus)
endif()

# Testing section
# ---------------

if(ARGENTUM_TESTS)
    # Let's use google test
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )

    # For Windows: Prevent overriding the parent project's compiler/linker settings
    # (borrowed from https://google.github.io/googletest/quickstart-cmake.html)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    include(GoogleTest)

    # target for testing
    # HEY!! TODO XXX: Change the name of the executable to the one you like!
    add_executable(argentum_tests)

    # HEY!! TODO XXX: I'm assuming that the code under testing is in taller_common
    # Change this if you have to.
    add_dependencies(argentum_tests taller_common taller_game taller_world)

    # Say where to find the xoz headers
    # HEY!! TODO XXX: add more subfolders here
    target_include_directories(argentum_tests PUBLIC .)

    # Add the source files to the argentum_tests target
    # HEY!! TODO XXX: add more subfolders here
    add_subdirectory(tests/)

    # Set the compiler warnings to the tests target (but be a little lesser strict)
    set_project_warnings(argentum_tests ${TALLER_MAKE_WARNINGS_AS_ERRORS} TRUE)
    taller_apply_sanitizers(argentum_tests)

    # Say where googletest headers can be found
    target_include_directories(argentum_tests
        PUBLIC
        ${gtest_SOURCE_DIR}/include
        ${gmock_SOURCE_DIR}/include
        )

    # HEY!! TODO XXX: I'm assuming that the code under testing is in taller_common
    # Change this if you have to.
    target_link_libraries(argentum_tests
        taller_common
        taller_game
        taller_world
        GTest::gtest_main
        spdlog::spdlog
    )
endif()
