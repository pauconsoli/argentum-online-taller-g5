#include "clan_panel.h"

#include <stdexcept>
#include <string>

ClanPanel::ClanPanel(SDL_Renderer* renderer, const std::string& font_path, int win_width,
                     int win_height):
    sdl_renderer(renderer), font(nullptr), window_width(win_width), window_height(win_height) {
    if (TTF_Init() < 0)
        throw std::runtime_error(TTF_GetError());
    font = TTF_OpenFont(font_path.c_str(), 14);
    if (font == nullptr) {
        TTF_Quit();
        throw std::runtime_error(TTF_GetError());
    }
}

ClanPanel::~ClanPanel() {
    TTF_CloseFont(font);
    TTF_Quit();
}

void ClanPanel::set_data(const ClanReviewUpdate& update) {
    clan_name_ = update.get_clan_name();
    members_ = update.get_members();
    pending_ = update.get_pending();
    visible_ = true;
}

void ClanPanel::hide() {
    visible_ = false;
}

bool ClanPanel::is_visible() const {
    return visible_;
}

void ClanPanel::draw_text(const std::string& text, int x, int y, SDL_Color color) {
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

void ClanPanel::draw() {
    if (!visible_)
        return;

    // filas: título + "Miembros:" + miembros + (opcional: "Pendientes:" + pendientes) + footer
    int content_rows = 2 + static_cast<int>(members_.size()) +
                       (pending_.empty() ? 0 : 1 + static_cast<int>(pending_.size())) + 1;
    int panel_h = content_rows * LINE_HEIGHT + 4 * PADDING;
    if (panel_h > window_height - 20)
        panel_h = window_height - 20;

    int panel_x = (window_width - PANEL_W) / 2;
    int panel_y = (window_height - panel_h) / 2;

    SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 30, 220);
    SDL_Rect panel = {panel_x, panel_y, PANEL_W, panel_h};
    SDL_RenderFillRect(sdl_renderer, &panel);

    SDL_SetRenderDrawColor(sdl_renderer, 180, 150, 80, 255);
    SDL_RenderDrawRect(sdl_renderer, &panel);

    SDL_Color title_color = {255, 220, 100, 255};
    SDL_Color header_color = {150, 200, 255, 255};
    SDL_Color online_color = {100, 255, 100, 255};
    SDL_Color offline_color = {160, 160, 160, 255};
    SDL_Color footer_color = {180, 180, 180, 255};

    int y = panel_y + PADDING;

    draw_text("Clan: " + clan_name_, panel_x + PADDING, y, title_color);
    y += LINE_HEIGHT + PADDING / 2;

    SDL_SetRenderDrawColor(sdl_renderer, 180, 150, 80, 180);
    SDL_RenderDrawLine(sdl_renderer, panel_x + PADDING, y - PADDING / 4,
                       panel_x + PANEL_W - PADDING, y - PADDING / 4);

    draw_text("Miembros (" + std::to_string(members_.size()) + "):", panel_x + PADDING, y,
              header_color);
    y += LINE_HEIGHT;

    for (const auto& m : members_) {
        std::string prefix = m.is_founder ? "[F] " : "    ";
        std::string label = prefix + m.nick + (m.is_online ? " (online)" : " (offline)");
        draw_text(label, panel_x + PADDING * 2, y, m.is_online ? online_color : offline_color);
        y += LINE_HEIGHT;
    }

    if (!pending_.empty()) {
        y += PADDING / 2;
        draw_text("Solicitudes pendientes (" + std::to_string(pending_.size()) + "):",
                  panel_x + PADDING, y, header_color);
        y += LINE_HEIGHT;
        for (const auto& p : pending_) {
            draw_text("  " + p.nick, panel_x + PADDING * 2, y, footer_color);
            y += LINE_HEIGHT;
        }
    }

    int footer_y = panel_y + panel_h - PADDING - LINE_HEIGHT;
    SDL_SetRenderDrawColor(sdl_renderer, 180, 150, 80, 100);
    SDL_RenderDrawLine(sdl_renderer, panel_x + PADDING, footer_y - PADDING / 4,
                       panel_x + PANEL_W - PADDING, footer_y - PADDING / 4);
    draw_text("[Esc] Cerrar", panel_x + PADDING, footer_y, footer_color);
}
