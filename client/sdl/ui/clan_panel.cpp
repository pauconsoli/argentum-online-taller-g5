#include "clan_panel.h"

#include <stdexcept>
#include <string>

namespace {
constexpr SDL_Color TITLE_COLOR = {255, 220, 100, 255};
constexpr SDL_Color HEADER_COLOR = {150, 200, 255, 255};
constexpr SDL_Color ONLINE_COLOR = {100, 255, 100, 255};
constexpr SDL_Color OFFLINE_COLOR = {160, 160, 160, 255};
constexpr SDL_Color FOOTER_COLOR = {180, 180, 180, 255};

constexpr SDL_Color PANEL_BG = {0, 0, 30, 220};
constexpr SDL_Color PANEL_BORDER = {180, 150, 80, 255};
}  // namespace

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

void ClanPanel::draw_line(const std::string& text, int x, int& y, SDL_Color color) {
    draw_text(text, x, y, color);
    y += LINE_HEIGHT;
}

void ClanPanel::draw_separator(int x1, int x2, int y, Uint8 alpha) {
    SDL_SetRenderDrawColor(sdl_renderer, PANEL_BORDER.r, PANEL_BORDER.g, PANEL_BORDER.b, alpha);
    SDL_RenderDrawLine(sdl_renderer, x1, y, x2, y);
}

int ClanPanel::count_content_rows() const {
    int rows = 2;  // titulo + encabezado "Miembros (N):"
    rows += static_cast<int>(members_.size());
    if (!pending_.empty())
        rows += 1 + static_cast<int>(pending_.size());
    rows += 1;  // footer
    return rows;
}

int ClanPanel::compute_panel_height() const {
    int height = count_content_rows() * LINE_HEIGHT + 4 * PADDING;
    int max_height = window_height - 20;
    return (height > max_height) ? max_height : height;
}

void ClanPanel::draw_background(const SDL_Rect& panel) {
    SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(sdl_renderer, PANEL_BG.r, PANEL_BG.g, PANEL_BG.b, PANEL_BG.a);
    SDL_RenderFillRect(sdl_renderer, &panel);

    SDL_SetRenderDrawColor(sdl_renderer, PANEL_BORDER.r, PANEL_BORDER.g, PANEL_BORDER.b,
                           PANEL_BORDER.a);
    SDL_RenderDrawRect(sdl_renderer, &panel);
}

void ClanPanel::draw_members(int x, int& y) {
    draw_line("Miembros (" + std::to_string(members_.size()) + "):", x, y, HEADER_COLOR);
    for (const auto& m : members_) {
        std::string prefix = m.is_founder ? "[F] " : "    ";
        std::string label = prefix + m.nick + (m.is_online ? " (online)" : " (offline)");
        draw_line(label, x + PADDING, y, m.is_online ? ONLINE_COLOR : OFFLINE_COLOR);
    }
}

void ClanPanel::draw_pending(int x, int& y) {
    if (pending_.empty())
        return;
    y += PADDING / 2;
    draw_line("Solicitudes pendientes (" + std::to_string(pending_.size()) + "):", x, y,
              HEADER_COLOR);
    for (const auto& p : pending_) draw_line("  " + p.nick, x + PADDING, y, FOOTER_COLOR);
}

void ClanPanel::draw() {
    if (!visible_)
        return;

    int panel_h = compute_panel_height();
    int panel_x = (window_width - PANEL_W) / 2;
    int panel_y = (window_height - panel_h) / 2;

    SDL_Rect panel = {panel_x, panel_y, PANEL_W, panel_h};
    draw_background(panel);

    int left = panel_x + PADDING;
    int right = panel_x + PANEL_W - PADDING;
    int y = panel_y + PADDING;

    draw_line("Clan: " + clan_name_, left, y, TITLE_COLOR);
    y += PADDING / 2;
    draw_separator(left, right, y - PADDING / 4, 180);

    draw_members(left, y);
    draw_pending(left, y);

    int footer_y = panel_y + panel_h - PADDING - LINE_HEIGHT;
    draw_separator(left, right, footer_y - PADDING / 4, 100);
    draw_text("[Esc] Cerrar", left, footer_y, FOOTER_COLOR);
}
