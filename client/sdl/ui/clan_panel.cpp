#include "clan_panel.h"

#include <stdexcept>
#include <string>

namespace {
constexpr SDL_Color TITLE_COLOR = {255, 220, 100, 255};    // dorado
constexpr SDL_Color HEADER_COLOR = {150, 200, 255, 255};   // celeste:
constexpr SDL_Color ONLINE_COLOR = {100, 255, 100, 255};   // verde
constexpr SDL_Color OFFLINE_COLOR = {160, 160, 160, 255};  // gris
constexpr SDL_Color FOOTER_COLOR = {180, 180, 180, 255};   // gris claro
constexpr SDL_Color PANEL_BG = {0, 0, 30, 220};            // azul semitransparente
constexpr SDL_Color PANEL_BORDER = {180, 150, 80, 255};    // borde dorado
}  // namespace

ClanPanel::ClanPanel(SDL_Renderer* renderer, const std::string& font_path, int win_width,
                     int win_height):
    sdl_renderer(renderer), font(nullptr), window_width(win_width), window_height(win_height) {
    // SDL_ttf ya fue inicializado por GameClient
    font = TTF_OpenFont(font_path.c_str(), 14);
    if (font == nullptr) {
        throw std::runtime_error(TTF_GetError());
    }
}

ClanPanel::~ClanPanel() {
    TTF_CloseFont(font);
}

// recibe un update del servidor con nombre del clan, lista de miembros y solicitudes pendientes
// y activa la visibilidad del panel
void ClanPanel::set_data(const ClanReviewUpdate& update) {
    clan_name_ = update.get_clan_name();
    members_ = update.get_members();  // vector de MemberInfo {nick, is_online, is_founder}
    pending_ = update.get_pending();  // vector de PendingInfo {nick}
    visible_ = true;                  // muestra el panel al recibir datos
}

// oculta el panel (por ej. cuando el usuario presiona Esc)
void ClanPanel::hide() {
    visible_ = false;
}

bool ClanPanel::is_visible() const {
    return visible_;
}

// renderiza una cadena UTF-8 en la posicion (x, y) con el color dado
// crea Surface -> Texture -> dibuja -> destruye todo (no cachea texturas)
void ClanPanel::draw_text(const std::string& text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (surface == nullptr)
        return;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(sdl_renderer, surface);
    SDL_FreeSurface(surface);  // la surface ya no se necesita una vez que hay texture
    if (texture == nullptr)
        return;
    int w, h;
    SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);  // obtiene el tamanio real del texto
    SDL_Rect dst = {x, y, w, h};
    SDL_RenderCopy(sdl_renderer, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
}

// dibuja una linea de texto y avanza y en LINE_HEIGHT para la siguiente
// `y` es pasado por referencia para que el llamador vea el nuevo cursor vertical
void ClanPanel::draw_line(const std::string& text, int x, int& y, SDL_Color color) {
    draw_text(text, x, y, color);
    y += LINE_HEIGHT;
}

// dibuja una linea horizontal de separacion con el color del borde y alpha variable
void ClanPanel::draw_separator(int x1, int x2, int y, Uint8 alpha) {
    SDL_SetRenderDrawColor(sdl_renderer, PANEL_BORDER.r, PANEL_BORDER.g, PANEL_BORDER.b, alpha);
    SDL_RenderDrawLine(sdl_renderer, x1, y, x2, y);
}

// cuenta cuantas filas de texto va a necesitar el panel para calcular su altura
int ClanPanel::count_content_rows() const {
    int rows = 2;                               // titulo del clan + encabezado "Miembros (N):"
    rows += static_cast<int>(members_.size());  // una fila por miembro
    if (!pending_.empty())
        rows += 1 + static_cast<int>(pending_.size());  // encabezado + una fila por solicitud
    rows += 1;                                          // footer "[Esc] Cerrar"
    return rows;
}

// calcula la altura del panel en pixeles; nunca supera la ventana menos 20px de margen
int ClanPanel::compute_panel_height() const {
    int height = count_content_rows() * LINE_HEIGHT + 4 * PADDING;
    int max_height = window_height - 20;
    return (height > max_height) ? max_height : height;
}

// rellena el rectangulo del panel con fondo semitransparente y luego dibuja el borde
void ClanPanel::draw_background(const SDL_Rect& panel) {
    SDL_SetRenderDrawBlendMode(sdl_renderer,
                               SDL_BLENDMODE_BLEND);  // necesario para la transparencia
    SDL_SetRenderDrawColor(sdl_renderer, PANEL_BG.r, PANEL_BG.g, PANEL_BG.b, PANEL_BG.a);
    SDL_RenderFillRect(sdl_renderer, &panel);  // relleno del fondo

    SDL_SetRenderDrawColor(sdl_renderer, PANEL_BORDER.r, PANEL_BORDER.g, PANEL_BORDER.b,
                           PANEL_BORDER.a);
    SDL_RenderDrawRect(sdl_renderer, &panel);  // borde del panel (solo contorno)
}

// dibuja la seccion de miembros: encabezado con conteo + una fila por miembro
// el fundador tiene prefijo "[F]", los demas tienen sangria; color segun online/offline
void ClanPanel::draw_members(int x, int& y) {
    draw_line("Miembros (" + std::to_string(members_.size()) + "):", x, y, HEADER_COLOR);
    for (const auto& m : members_) {
        std::string prefix = m.is_founder ? "[F] " : "    ";
        std::string label = prefix + m.nick + (m.is_online ? " (online)" : " (offline)");
        draw_line(label, x + PADDING, y, m.is_online ? ONLINE_COLOR : OFFLINE_COLOR);
    }
}

// dibuja la seccion de solicitudes pendientes; si no hay ninguna, no dibuja nada
void ClanPanel::draw_pending(int x, int& y) {
    if (pending_.empty())
        return;
    y += PADDING / 2;  // espacio extra antes del header de pendientes
    draw_line("Solicitudes pendientes (" + std::to_string(pending_.size()) + "):", x, y,
              HEADER_COLOR);
    for (const auto& p : pending_) draw_line("  " + p.nick, x + PADDING, y, FOOTER_COLOR);
}

// punto de entrada del renderizado: no dibuja nada si el panel esta oculto
void ClanPanel::draw() {
    if (!visible_)
        return;

    // calcula posicion del panel centrado en la ventana
    int panel_h = compute_panel_height();
    int panel_x = (window_width - PANEL_W) / 2;
    int panel_y = (window_height - panel_h) / 2;

    SDL_Rect panel = {panel_x, panel_y, PANEL_W, panel_h};
    draw_background(panel);

    int left = panel_x + PADDING;             // margen izquierdo del texto
    int right = panel_x + PANEL_W - PADDING;  // margen derecho para los separadores
    int y = panel_y + PADDING;  // cursor vertical, empieza debajo del padding superior

    // Titulo
    draw_line("Clan: " + clan_name_, left, y, TITLE_COLOR);
    y += PADDING / 2;                                   // espacio extra debajo del titulo
    draw_separator(left, right, y - PADDING / 4, 180);  // linea divisoria semi-opaca

    // Cuerpo: miembros y solicitudes pendientes
    draw_members(left, y);
    draw_pending(left, y);

    // Footer fijo en el borde inferior del panel, independiente del contenido
    int footer_y = panel_y + panel_h - PADDING - LINE_HEIGHT;
    draw_separator(left, right, footer_y - PADDING / 4, 100);  // separador tenue antes del footer
    draw_text("[Esc] Cerrar", left, footer_y, FOOTER_COLOR);
}
