#ifndef HELP_MENU_H  // guard para evitar doble inclusion del header
#define HELP_MENU_H

#include <string>   // std::string
#include <utility>  // std::pair
#include <vector>   // std::vector

#include <SDL2/SDL.h>  // SDL_Renderer, SDL_Color
#include <SDL_ttf.h>   // TTF_Font, renderizado de texto

class HelpMenu {
 private:
    SDL_Renderer* sdl_renderer;  // renderer de SDL al que se dibuja (no owneado, raw ptr)
    TTF_Font* font;              // fuente cargada con SDL_ttf para el texto del menu
    int window_width;            // ancho de la ventana, para centrar el panel
    int window_height;           // alto de la ventana, para posicionar el panel

    bool visible;  // si el menu esta abierto o cerrado; toggle() lo cambia

    // cada par es (tecla, descripcion), ej: {"F1", "Abrir ayuda"}
    std::vector<std::pair<std::string, std::string>> left;    // columna izquierda de keybindings
    std::vector<std::pair<std::string, std::string>> right;   // columna derecha de keybindings
    std::vector<std::pair<std::string, std::string>> cheats;  // seccion de cheats/comandos extra

    // helper interno: renderiza `text` en la posicion (x, y) con el color dado
    void draw_text(const std::string& text, int x, int y, SDL_Color color);

 public:
    // carga la fuente desde font_path y guarda dimensiones de ventana
    HelpMenu(SDL_Renderer* renderer, const std::string& font_path, int win_width, int win_height);
    ~HelpMenu();  // libera la fuente con TTF_CloseFont

    HelpMenu(const HelpMenu&) = delete;             // no se puede copiar (owna la fuente)
    HelpMenu& operator=(const HelpMenu&) = delete;  // idem

    void toggle();  // abre si estaba cerrado, cierra si estaba abierto
    void draw();    // dibuja el panel completo solo si visible == true
};

#endif
