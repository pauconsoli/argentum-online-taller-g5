#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <SDL2/SDL.h>

class InputHandler {
 public:
    InputHandler();

    // Retorna false si el usuario cerró la ventana, true si no
    bool handle(const SDL_Event& event);
};

#endif
