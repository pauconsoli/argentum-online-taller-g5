#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <SDL2/SDL.h>

// Por ahora solo identifica el caso que el usuario cierra la ventana
class InputHandler {
 public:
    InputHandler();

    // retorna false si el usuario cierra la ventana, true si no
    static bool handle(const SDL_Event& event);
};

#endif
