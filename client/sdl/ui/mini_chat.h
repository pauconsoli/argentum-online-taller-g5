#ifndef MINI_CHAT_H
#define MINI_CHAT_H

#include <deque>  // std::deque: estructura de doble extremo, permite agregar atras y borrar adelante
#include <string>

#include <SDL2/SDL.h>  // SDL_Renderer, SDL_Color, SDL_Rect
#include <SDL_ttf.h>   // TTF_Font, renderizado de texto con fuente

class MiniChat {
 private:
    static constexpr int MAX_MESSAGES = 8;  // cuantos mensajes se muestran como maximo a la vez
    static constexpr int LINE_HEIGHT = 18;  // altura en px de cada linea de texto
    static constexpr int PADDING = 5;       // margen interior del cuadro de mensajes/input

    SDL_Renderer* sdl_renderer;  // renderer al que se dibuja (no owneado)
    TTF_Font* font;    // fuente para renderizar texto (si es owned, se libera en destructor)
    int window_width;  // ancho de la ventana; el chat ocupa el 60% de este valor

    std::deque<std::string> messages;  // cola de mensajes; el mas viejo se borra del frente

    // helper privado: renderiza `text` en (x, y) con el color dado
    void draw_text(const std::string& text, int x, int y, SDL_Color color);

 public:
    // inicializa SDL_ttf y carga la fuente de 14px desde font_path
    MiniChat(SDL_Renderer* renderer, const std::string& font_path, int win_width);
    ~MiniChat();  // cierra fuente y apaga SDL_ttf

    MiniChat(const MiniChat&) = delete;             // no se puede copiar (owna la fuente)
    MiniChat& operator=(const MiniChat&) = delete;  // idem

    // agrega un mensaje al final de la cola; si supera MAX_MESSAGES, descarta el mas viejo
    void add_message(const std::string& msg);

    // dibuja la franja semitransparente con todos los mensajes en la esquina superior izquierda
    void draw();

    // dibuja el cuadro de entrada de texto con el contenido actual del input y un cursor "|"
    void draw_input(const std::string& input, int y);
};

#endif
