#include "input_handler.h"

std::vector<InputAction> InputHandler::poll_events(const InputContext& context) {
    std::vector<InputAction> actions;
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            actions.push_back({InputActionType::Quit});
            continue;
        }

        // flotante
        if (context.overlay_visible &&
            ((event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) ||
             (event.type == SDL_MOUSEBUTTONDOWN))) {
            actions.push_back({InputActionType::DismissOverlay});
            continue;
        }

        if (context.chat_active) {
            if (event.type == SDL_TEXTINPUT) {  // texto
                InputAction action_input{InputActionType::ChatText};
                action_input.text = event.text.text;
                actions.push_back(action_input);
            } else if (event.type == SDL_KEYDOWN) {  // presionar tecla
                switch (event.key.keysym.sym) {
                    case SDLK_RETURN:
                    case SDLK_RETURN2:
                        actions.push_back({InputActionType::ChatSubmit});
                        break;
                    case SDLK_BACKSPACE:
                        actions.push_back({InputActionType::ChatBackspace});
                        break;
                    case SDLK_ESCAPE:
                        actions.push_back({InputActionType::ChatCancel});
                        break;
                    default:
                        break;
                }
            }
            continue;
        }

        // presionar teclas
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) {
            actions.push_back({InputActionType::OpenChat});
            continue;
        }
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_h) {
            actions.push_back({InputActionType::ToggleHelp});
            continue;
        }
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_m) {
            actions.push_back({InputActionType::ToggleMusic});
            continue;
        }
        // cheats
        if (event.type == SDL_KEYDOWN && (event.key.keysym.mod & KMOD_CTRL) &&
            event.key.repeat == 0) {
            int ability_slot = 0;
            switch (event.key.keysym.sym) {
                case SDLK_1:
                    ability_slot = 1;
                    break;
                case SDLK_2:
                    ability_slot = 2;
                    break;
                case SDLK_3:
                    ability_slot = 3;
                    break;
                case SDLK_4:
                    ability_slot = 4;
                    break;
                case SDLK_5:
                    ability_slot = 5;
                    break;
                default:
                    break;
            }
            if (ability_slot > 0) {
                InputAction ability_action{InputActionType::Cheat};
                ability_action.cheat_slot = ability_slot;
                actions.push_back(ability_action);
            }
            continue;
        }  // clicks
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            InputAction left_click_action{InputActionType::ClickPrimary};
            left_click_action.x = event.button.x;
            left_click_action.y = event.button.y;
            actions.push_back(left_click_action);
            continue;
        }
        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT) {
            InputAction right_click_action{InputActionType::ClickSecondary};
            right_click_action.x = event.button.x;
            right_click_action.y = event.button.y;
            actions.push_back(right_click_action);
            continue;
        }
    }

    return actions;
}

MoveDir InputHandler::poll_movement(bool chat_active) {
    if (chat_active)
        return MoveDir::None;
    const Uint8* keyboard_state = SDL_GetKeyboardState(nullptr);
    if (keyboard_state[SDL_SCANCODE_DOWN])
        return MoveDir::Down;
    if (keyboard_state[SDL_SCANCODE_UP])
        return MoveDir::Up;
    if (keyboard_state[SDL_SCANCODE_LEFT])
        return MoveDir::Left;
    if (keyboard_state[SDL_SCANCODE_RIGHT])
        return MoveDir::Right;
    return MoveDir::None;
}
