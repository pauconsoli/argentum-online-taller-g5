#include "mini_chat.h"

#include <stdexcept>

// carga la fuente (SDL_ttf ya fue inicializado por GameClient)
MiniChat::MiniChat(SDL_Renderer* renderer, const std::string& font_path, int win_width):
    sdl_renderer(renderer), font(nullptr), window_width(win_width) {
    font = TTF_OpenFont(font_path.c_str(), 14);
    if (font == nullptr) {
        throw std::runtime_error(TTF_GetError());
    }
}

MiniChat::~MiniChat() {
    TTF_CloseFont(font);
}

// renderiza una cadena UTF-8 en (x, y): Surface → Texture → dibuja → destruye
// no cachea texturas; cada llamada crea y destruye su propia textura
void MiniChat::draw_text(const std::string& text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (surface == nullptr)
        return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(sdl_renderer, surface);
    SDL_FreeSurface(surface);  // la surface ya no se usa una vez convertida a textura
    if (texture == nullptr)
        return;

    int w, h;
    SDL_QueryTexture(texture, nullptr, nullptr, &w,
                     &h);  // obtiene ancho/alto del texto renderizado
    SDL_Rect dst = {x, y, w, h};
    SDL_RenderCopy(sdl_renderer, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
}

// agrega un mensaje a la cola; si ya hay MAX_MESSAGES, descarta el mas viejo (pop_front)
void MiniChat::add_message(const std::string& msg) {
    messages.push_back(msg);
    while (static_cast<int>(messages.size()) > MAX_MESSAGES) {
        messages.pop_front();  // el deque actua como buffer circular de capacidad fija
    }
}

// dibuja el cuadro de entrada de texto en la fila y dada
// el texto se muestra como "> <input>|" (el "|" simula el cursor del teclado)
void MiniChat::draw_input(const std::string& input, int y) {
    const std::string display = "> " + input + "|";
    int box_w = window_width * 6 / 10;  // el cuadro ocupa el 60% del ancho de la ventana
    SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 200);  // negro con 78% de opacidad
    SDL_Rect box = {0, y, box_w, LINE_HEIGHT + 2 * PADDING};
    SDL_RenderFillRect(sdl_renderer, &box);  // rellena el fondo del cuadro
    SDL_Color white = {255, 255, 255, 255};
    draw_text(display, 10, y + PADDING, white);  // dibuja el texto con margen izquierdo de 10px
}

// dibuja la franja de mensajes en la esquina superior izquierda de la pantalla
// si no hay mensajes, no dibuja nada
void MiniChat::draw() {
    if (messages.empty())
        return;

    SDL_Color color = {255, 220, 100, 255};  // dorado: mismo tono que el resto de la UI
    int strip_w = window_width * 6 / 10;     // franja ocupa el 60% del ancho
    int strip_h = static_cast<int>(messages.size()) * LINE_HEIGHT + 2 * PADDING;  // altura dinamica

    SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 180);  // negro con 70% de opacidad
    SDL_Rect strip = {0, 0, strip_w, strip_h};
    SDL_RenderFillRect(sdl_renderer, &strip);  // fondo semitransparente detras del texto

    int y = PADDING;  // empieza con el margen superior
    for (const auto& msg : messages) {
        draw_text(msg, 10, y, color);  // margen izquierdo fijo de 10px
        y += LINE_HEIGHT;              // avanza una fila
    }
}
