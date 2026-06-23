#include "help_menu.h"

#include <stdexcept>

HelpMenu::HelpMenu(SDL_Renderer* renderer, const std::string& font_path, int win_width,
                   int win_height):
    sdl_renderer(renderer),
    font(nullptr),
    window_width(win_width),
    window_height(win_height),
    visible(false) {
    if (TTF_Init() < 0) {
        throw std::runtime_error(TTF_GetError());
    }
    font = TTF_OpenFont(font_path.c_str(), 14);
    if (font == nullptr) {
        throw std::runtime_error(TTF_GetError());
    }

    left = {
        {"Controles", ""},
        {"Flechas", "Mover el personaje"},
        {"Click izq. jugador/NPC", "Atacar al objetivo"},
        {"Click izq. inventario", "Equipar el item"},
        {"Click der. inventario", "Seleccionar item"},
        {"Enter", "Abrir / enviar chat"},
        {"Esc", "Cancelar el chat"},
        {"H", "Abrir / cerrar ayuda"},
        {"", ""},
        {"Chat (Enter y escribir)", ""},
        {"/meditar", "Recuperar maná"},
        {"/resucitar", "Resucitar"},
        {"/curar", "Curar vida y maná"},
        {"/tomar", "Levantar del suelo"},
        {"/tirar", "Tirar el item"},
        {"@nick msg", "Mensaje privado"},
    };

    right = {
        {"Comercio y banco", ""},
        {"/listar", "Ver catálogo/bóveda"},
        {"/comprar <obj>", "Comprar item"},
        {"/vender <obj>", "Vender item"},
        {"/depositar <obj>", "Guardar en el banco"},
        {"/depositar oro <cant>", " "},
        {"/retirar <obj>", "Sacar del banco"},
        {"/retirar oro <cant>", " "},
        {"", ""},
        {"Clanes (fundador)", ""},
        {"/fundar-clan <n>", "Fundar un clan"},
        {"/unirse <n>", "Pedir unirse"},
        {"/revisar-clan", "Ver pedidos/miembros"},
        {"/clan-aceptar <nick>", "Aceptar postulante"},
        {"/clan-rechazar <nick>", "Rechazar postulante"},
        {"/clan-ban <nick>", "Banear miembro"},
        {"/clan-kick <nick>", "Expulsar miembro"},
        {"/dejar-clan", "Salir del clan"},
    };

    cheats = {
        {"Cheats (para probar)", ""},   {"Ctrl + 1", "Vida al máximo"},
        {"Ctrl + 2", "Maná al máximo"}, {"Ctrl + 3", "Morir"},
        {"Ctrl + 4", "Subir de nivel"}, {"Ctrl + 5", "+10000 de oro"},
    };
}

HelpMenu::~HelpMenu() {
    TTF_CloseFont(font);
    TTF_Quit();
}

void HelpMenu::toggle() {
    visible = !visible;
}

bool HelpMenu::is_visible() const {
    return visible;
}

void HelpMenu::draw_text(const std::string& text, int x, int y, SDL_Color color) {
    if (text.empty())
        return;
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (surface == nullptr)
        return;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(sdl_renderer, surface);
    SDL_FreeSurface(surface);
    if (texture == nullptr)
        return;
    int w, h;
    SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
    SDL_Rect dst = {x, y, w, h};
    SDL_RenderCopy(sdl_renderer, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
}

void HelpMenu::draw() {
    if (!visible)
        return;

    SDL_Color yellow = {255, 220, 100, 255};
    SDL_Color blue = {120, 200, 255, 255};
    SDL_Color white = {225, 225, 225, 255};

    int rows = static_cast<int>(left.size());
    if (static_cast<int>(right.size()) > rows)
        rows = static_cast<int>(right.size());

    int panel_w = window_width * 90 / 100;
    int cheat_rows = 1 + static_cast<int>(cheats.size()) / 2;
    int panel_h = 60 + rows * 22 + 16 + cheat_rows * 22 + 66;
    int panel_x = (window_width - panel_w) / 2;
    int panel_y = (window_height - panel_h) / 2;

    // Fondo y borde del panel.
    SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(sdl_renderer, 8, 10, 30, 225);
    SDL_Rect panel = {panel_x, panel_y, panel_w, panel_h};
    SDL_RenderFillRect(sdl_renderer, &panel);
    SDL_SetRenderDrawColor(sdl_renderer, 255, 220, 100, 255);
    SDL_RenderDrawRect(sdl_renderer, &panel);

    draw_text("Comandos disponibles", panel_x + 20, panel_y + 20, yellow);

    int col1 = panel_x + 20;
    int col2 = panel_x + panel_w / 2;
    int desc = 200;  // distancia del comando a su descripcion

    // Columna izquierda.
    int y = panel_y + 60;
    for (auto& line : left) {
        if (line.second.empty()) {
            draw_text(line.first, col1, y, yellow);
        } else {
            draw_text(line.first, col1, y, blue);
            draw_text(line.second, col1 + desc, y, white);
        }
        y += 22;
    }

    // Columna derecha.
    y = panel_y + 60;
    for (auto& line : right) {
        if (line.second.empty()) {
            draw_text(line.first, col2, y, yellow);
        } else {
            draw_text(line.first, col2, y, blue);
            draw_text(line.second, col2 + desc, y, white);
        }
        y += 22;
    }

    // Cheats en 2 columnas, debajo de las columnas principales.
    y = panel_y + 60 + rows * 22 + 16;
    if (!cheats.empty()) {
        draw_text(cheats[0].first, col1, y, yellow);
        y += 22;
        for (int i = 1; i < static_cast<int>(cheats.size()); i += 2) {
            draw_text(cheats[i].first, col1, y, blue);
            draw_text(cheats[i].second, col1 + desc, y, white);
            if (i + 1 < static_cast<int>(cheats.size())) {
                draw_text(cheats[i + 1].first, col2, y, blue);
                draw_text(cheats[i + 1].second, col2 + desc, y, white);
            }
            y += 22;
        }
    }

    draw_text("Algunos comandos requieren seleccionar antes al NPC o jugador.", col1, y + 6, white);
    draw_text("El nick de los miembros de tu clan aparecerá en color verde.", col1, y + 28, white);
}
