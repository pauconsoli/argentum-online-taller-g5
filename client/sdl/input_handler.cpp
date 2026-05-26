#include "input_handler.h"
#include <iostream>

InputHandler::InputHandler() {}

bool InputHandler::handle(SDL_Event& event) {
    if (event.type == SDL_QUIT) {
        return false;
    }

    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_UP:
                std::cout << "Mover arriba" << std::endl;
                break;
            case SDLK_DOWN:
                std::cout << "Mover abajo" << std::endl;
                break;
            case SDLK_LEFT:
                std::cout << "Mover izquierda" << std::endl;
                break;
            case SDLK_RIGHT:
                std::cout << "Mover derecha" << std::endl;
                break;
            default:
                break;
        }
    }

    return true;
}