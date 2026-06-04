#include "input_handler.h"

InputHandler::InputHandler() {}

bool InputHandler::handle(const SDL_Event& event) {
    if (event.type == SDL_QUIT) {
        return false;
    }
    return true;
}
