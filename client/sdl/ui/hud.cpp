#include "hud.h"

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "../render/renderer.h"

// carga fuente de 16px; lanza excepcion si falla (SDL_ttf ya fue inicializado por GameClient)
Hud::Hud(SDL_Renderer* renderer, const std::string& font_path, int win_height, int win_width):
    sdl_renderer(renderer), font(nullptr), window_height(win_height), window_width(win_width) {
    font = TTF_OpenFont(font_path.c_str(), 16);
    if (font == nullptr) {
        throw std::runtime_error(TTF_GetError());
    }
}

// destruye todas las texturas cacheadas antes de cerrar la fuente
Hud::~Hud() {
    for (auto& c : text_caches_)
        if (c.texture)
            SDL_DestroyTexture(c.texture);
    TTF_CloseFont(font);
}

// comprueba si la textura del slot sigue siendo valida para el texto y color actuales
// si algo cambio (o nunca se genero), destruye la vieja y renderiza una nueva
SDL_Texture* Hud::ensure_cached(int slot, const std::string& text, SDL_Color color) {
    TextCache& c = text_caches_[slot];
    bool stale = !c.texture || c.last_text != text || c.last_color.r != color.r ||
                 c.last_color.g != color.g || c.last_color.b != color.b ||
                 c.last_color.a != color.a;
    if (stale) {
        if (c.texture)
            SDL_DestroyTexture(c.texture);  // descarta textura desactualizada
        SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
        c.texture = surface ? SDL_CreateTextureFromSurface(sdl_renderer, surface) : nullptr;
        c.last_text = text;
        c.last_color = color;
        if (surface)
            SDL_FreeSurface(surface);
    }
    return c.texture;
}

// recupera la textura del cache y la dibuja en (x, y) con su tamanio natural
void Hud::draw_cached_text(int slot, const std::string& text, int x, int y, SDL_Color color) {
    SDL_Texture* tex = ensure_cached(slot, text, color);
    if (!tex)
        return;
    int w, h;
    SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);
    SDL_Rect dst = {x, y, w, h};
    SDL_RenderCopy(sdl_renderer, tex, nullptr, &dst);
}

// dibuja una barra de progreso de ancho `w` y alto `h` en (x, y):
//   1. fondo gris oscuro semitransparente
//   2. relleno proporcional a current/max_val con el color dado
//   3. texto "current/max_val" centrado sobre la barra
void Hud::draw_bar(int x, int y, int w, int h, int current, int max_val, SDL_Color color,
                   int text_slot) {
    SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(sdl_renderer, 45, 45, 45, 210);  // gris oscuro semitransparente
    SDL_Rect bg = {x, y, w, h};
    SDL_RenderFillRect(sdl_renderer, &bg);

    if (max_val > 0 && current > 0) {
        int filled = w * current / max_val;  // pixeles a rellenar proporcionales al valor actual
        SDL_SetRenderDrawColor(sdl_renderer, color.r, color.g, color.b, color.a);
        SDL_Rect fill = {x, y, filled, h};
        SDL_RenderFillRect(sdl_renderer, &fill);
    }

    // texto "actual/maximo" centrado horizontalmente y verticalmente dentro de la barra
    std::string label = std::to_string(current) + "/" + std::to_string(max_val);
    SDL_Color white = {255, 255, 255, 255};
    SDL_Texture* tex = ensure_cached(text_slot, label, white);
    if (tex) {
        int tw, th;
        SDL_QueryTexture(tex, nullptr, nullptr, &tw, &th);
        SDL_Rect dst = {x + (w - tw) / 2, y + (h - th) / 2, tw, th};  // centrado
        SDL_RenderCopy(sdl_renderer, tex, nullptr, &dst);
    }
}

// dibuja el panel de inventario 5x4 en la esquina superior derecha de la ventana
void Hud::draw_inventory(SpriteManager* sprites, const std::vector<InventorySlotData>& slots,
                         int selected_slot) {
    constexpr int COLS = 5;      // columnas del inventario
    constexpr int ROWS = 4;      // filas del inventario
    constexpr int CELL = 40;     // tamanio de cada celda en px
    constexpr int GAP = 2;       // separacion entre celdas
    constexpr int PAD = 8;       // padding interno del panel
    constexpr int TITLE_H = 24;  // altura reservada para el titulo "Inventario"
    constexpr int MARGIN = 5;    // margen desde el borde de la ventana
    constexpr int ICON = 32;     // tamanio del icono del item dentro de la celda

    const int grid_w = COLS * CELL + (COLS - 1) * GAP;  // ancho total de la grilla
    const int grid_h = ROWS * CELL + (ROWS - 1) * GAP;  // alto total de la grilla
    const int panel_w = grid_w + 2 * PAD;
    const int panel_h = PAD + TITLE_H + grid_h + PAD;
    const int panel_x = window_width - panel_w - MARGIN;  // esquina derecha
    const int panel_y = MARGIN;                           // esquina superior

    SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);

    // fondo del panel
    SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 170);
    SDL_Rect panel = {panel_x, panel_y, panel_w, panel_h};
    SDL_RenderFillRect(sdl_renderer, &panel);

    SDL_Color white = {255, 255, 255, 255};
    draw_cached_text(TCACHE_INV_TITLE, "Inventario", panel_x + PAD, panel_y + PAD / 2, white);

    const int grid_x = panel_x + PAD;            // origen x de la grilla
    const int grid_y = panel_y + PAD + TITLE_H;  // origen y de la grilla (debajo del titulo)

    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < COLS; ++col) {
            int cell_x = grid_x + col * (CELL + GAP);
            int cell_y = grid_y + row * (CELL + GAP);
            int slot = row * COLS + col;  // indice lineal 0-19

            // fondo de la celda
            SDL_SetRenderDrawColor(sdl_renderer, 55, 55, 55, 255);
            SDL_Rect fill = {cell_x, cell_y, CELL, CELL};
            SDL_RenderFillRect(sdl_renderer, &fill);

            // cppcheck-suppress syntaxError
            const int n_slots = static_cast<int>(slots.size());
            bool occupied = (slot < n_slots) && !slots[slot].item_name.empty();  // slot tiene item
            bool equipped = occupied && slots[slot].is_equipped;                 // item equipado

            // borde amarillo si el item esta equipado, gris si no
            if (equipped)
                SDL_SetRenderDrawColor(sdl_renderer, 255, 220, 0, 255);
            else
                SDL_SetRenderDrawColor(sdl_renderer, 110, 110, 110, 255);
            SDL_RenderDrawRect(sdl_renderer, &fill);

            // borde celeste exterior adicional si el slot esta seleccionado con click derecho
            if (slot == selected_slot) {
                SDL_Rect sel = {cell_x - 1, cell_y - 1, CELL + 2, CELL + 2};
                SDL_SetRenderDrawColor(sdl_renderer, 0, 200, 255, 255);
                SDL_RenderDrawRect(sdl_renderer, &sel);
            }

            if (sprites != nullptr && occupied) {
                // obtiene la textura del icono del item por su nombre
                std::string item_key = SpriteManager::item_key_for_name(slots[slot].item_name);
                SDL_Texture* icon = sprites->get_item(item_key);
                if (icon != nullptr) {
                    int ox = cell_x + (CELL - ICON) / 2;  // centra el icono en la celda
                    int oy = cell_y + (CELL - ICON) / 2;
                    SDL_Rect dst = {ox, oy, ICON, ICON};

                    // contorno negro: se dibuja el mismo icono 8 veces desplazado 1px en cada
                    // direccion misma tecnica que Renderer::draw_frame_scaled_outlined
                    static const int dirs[8][2] = {{-1, 0},  {1, 0},  {0, -1}, {0, 1},
                                                   {-1, -1}, {1, -1}, {-1, 1}, {1, 1}};
                    SDL_SetTextureColorMod(icon, 0, 0, 0);  // tiñe el icono de negro
                    SDL_SetTextureAlphaMod(icon, Renderer::OUTLINE_ALPHA);
                    for (const auto& d : dirs) {
                        SDL_Rect off = {ox + d[0] * Renderer::OUTLINE_THICKNESS,
                                        oy + d[1] * Renderer::OUTLINE_THICKNESS, ICON, ICON};
                        SDL_RenderCopy(sdl_renderer, icon, nullptr, &off);
                    }
                    // restaura el icono a su color y alpha originales antes de dibujarlo encima
                    SDL_SetTextureAlphaMod(icon, 255);
                    SDL_SetTextureColorMod(icon, 255, 255, 255);
                    SDL_RenderCopy(sdl_renderer, icon, nullptr, &dst);
                }

                // cantidad en la esquina inferior derecha de la celda, solo si qty > 1
                uint32_t qty = slots[slot].quantity;
                if (qty > 1) {
                    std::string qty_str = std::to_string(qty);
                    SDL_Color yellow = {255, 220, 0, 255};
                    SDL_Texture* qty_tex =
                        ensure_cached(TCACHE_INV_QTY_BASE + slot, qty_str, yellow);
                    if (qty_tex) {
                        int tw, th;
                        SDL_QueryTexture(qty_tex, nullptr, nullptr, &tw, &th);
                        // pegado al borde inferior derecho de la celda
                        SDL_Rect dst = {cell_x + CELL - tw - 2, cell_y + CELL - th, tw, th};
                        SDL_RenderCopy(sdl_renderer, qty_tex, nullptr, &dst);
                    }
                }
            }
        }
    }
}

// dado un punto de pantalla, calcula en cual slot del inventario cae
// tiene en cuenta el GAP entre celdas: si el punto cae en el gap, devuelve -1
int Hud::get_slot_at(int screen_x, int screen_y) const {
    constexpr int COLS = 5;
    constexpr int ROWS = 4;
    constexpr int CELL = 40;
    constexpr int GAP = 2;
    constexpr int PAD = 8;
    constexpr int TITLE_H = 24;
    constexpr int MARGIN = 5;

    const int grid_w = COLS * CELL + (COLS - 1) * GAP;
    const int panel_w = grid_w + 2 * PAD;
    const int panel_x = window_width - panel_w - MARGIN;
    const int panel_y = MARGIN;

    const int grid_x = panel_x + PAD;
    const int grid_y = panel_y + PAD + TITLE_H;

    int rel_x = screen_x - grid_x;  // coordenada relativa al origen de la grilla
    int rel_y = screen_y - grid_y;

    if (rel_x < 0 || rel_y < 0)
        return -1;  // el punto esta a la izquierda o arriba de la grilla

    int col = rel_x / (CELL + GAP);
    int row = rel_y / (CELL + GAP);

    if (col >= COLS || row >= ROWS)
        return -1;  // el punto esta fuera de la grilla

    // verifica que el punto no caiga en el gap entre celdas
    if (rel_x % (CELL + GAP) >= CELL || rel_y % (CELL + GAP) >= CELL)
        return -1;

    return row * COLS + col;
}

// dibuja el boton de musica en la esquina inferior derecha
// si `paused` es true → rojo + "OFF"; si no → verde + "ON"
void Hud::draw_music_button(bool paused) {
    constexpr int BTN_W = 140;
    constexpr int BTN_H = 24;
    constexpr int MARGIN = 5;
    const int btn_x = window_width - BTN_W - MARGIN;
    const int btn_y = window_height - BTN_H - MARGIN;

    SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 160);  // fondo negro semitransparente
    SDL_Rect bg = {btn_x, btn_y, BTN_W, BTN_H};
    SDL_RenderFillRect(sdl_renderer, &bg);

    SDL_Color color = paused ? SDL_Color{180, 60, 60, 255} : SDL_Color{60, 200, 100, 255};
    std::string label = paused ? "[M] Musica: OFF" : "[M] Musica: ON";
    draw_cached_text(TCACHE_MUSIC_BTN, label, btn_x + 5, btn_y + 4, color);
}

// devuelve el SDL_Rect del boton de musica para que el input handler detecte clicks sobre el
SDL_Rect Hud::get_music_button_rect() const {
    constexpr int BTN_W = 140;
    constexpr int BTN_H = 24;
    constexpr int MARGIN = 5;
    return {window_width - BTN_W - MARGIN, window_height - BTN_H - MARGIN, BTN_W, BTN_H};
}

// dibuja el panel de stats en la esquina inferior izquierda:
// nivel, barra de HP, barra de MP, oro y experiencia
void Hud::draw(int hp, int max_hp, int mana, int max_mana, int level, uint64_t gold, uint64_t xp) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color yellow = {255, 220, 60, 255};
    SDL_Color gray = {180, 180, 180, 255};
    SDL_Color red = {220, 55, 55, 255};
    SDL_Color blue = {55, 100, 255, 255};

    constexpr int LABEL_W = 30;  // ancho reservado para la etiqueta "HP " / "MP "
    constexpr int BAR_H = 18;    // alto de las barras de vida y mana
    constexpr int ROW_GAP = 4;   // espacio vertical entre filas
    constexpr int PAD = 6;       // padding interno del panel
    constexpr int margin = 5;    // margen desde el borde de la ventana

    // altura total del panel calculada sumando cada fila
    constexpr int panel_w = 190;
    constexpr int panel_h = PAD + 20 + ROW_GAP  // fila: Nivel
                            + BAR_H + ROW_GAP   // fila: barra HP
                            + BAR_H + ROW_GAP   // fila: barra MP
                            + 18 + ROW_GAP      // fila: Oro
                            + 18 + PAD;         // fila: Exp + padding inferior
    int panel_x = margin;
    int panel_y = window_height - panel_h - margin;  // pegado al borde inferior

    // fondo del panel semitransparente
    SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 160);
    SDL_Rect panel = {panel_x, panel_y, panel_w, panel_h};
    SDL_RenderFillRect(sdl_renderer, &panel);

    int cy = panel_y + PAD;  // cursor vertical, avanza fila a fila

    // — Nivel —
    draw_cached_text(TCACHE_NIVEL, "Nivel: " + std::to_string(level), panel_x + PAD, cy, white);
    cy += 20 + ROW_GAP;

    // — Vida: etiqueta "HP" a la izquierda + barra roja a la derecha de la etiqueta —
    draw_cached_text(TCACHE_HP_LABEL, "HP", panel_x + PAD, cy + 1, red);
    draw_bar(panel_x + PAD + LABEL_W, cy, panel_w - PAD * 2 - LABEL_W, BAR_H, hp, max_hp, red,
             TCACHE_HP_BAR);
    cy += BAR_H + ROW_GAP;

    // — Maná: misma estructura que vida pero en azul —
    draw_cached_text(TCACHE_MP_LABEL, "MP", panel_x + PAD, cy + 1, blue);
    draw_bar(panel_x + PAD + LABEL_W, cy, panel_w - PAD * 2 - LABEL_W, BAR_H, mana, max_mana, blue,
             TCACHE_MP_BAR);
    cy += BAR_H + ROW_GAP;

    // — Oro —
    draw_cached_text(TCACHE_ORO, "Oro:  " + std::to_string(gold), panel_x + PAD, cy, yellow);
    cy += 18 + ROW_GAP;

    // — Experiencia —
    draw_cached_text(TCACHE_EXP, "Exp:  " + std::to_string(xp), panel_x + PAD, cy, gray);
}
