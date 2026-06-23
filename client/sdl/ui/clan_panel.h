#ifndef CLAN_PANEL_H
#define CLAN_PANEL_H

#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL_ttf.h>

#include "common/clan/clan_review_result.h"
#include "common/updates/clan_review_update.h"

class ClanPanel {
 private:
    static constexpr int LINE_HEIGHT = 16;
    static constexpr int PADDING = 12;
    static constexpr int PANEL_W = 380;

    SDL_Renderer* sdl_renderer;
    TTF_Font* font;
    int window_width;
    int window_height;
    bool visible_ = false;
    std::string clan_name_;
    std::vector<ClanMemberInfo> members_;
    std::vector<ClanMemberInfo> pending_;

    void draw_text(const std::string& text, int x, int y, SDL_Color color);
    void draw_line(const std::string& text, int x, int& y, SDL_Color color);
    void draw_separator(int x1, int x2, int y, Uint8 alpha);
    void draw_background(const SDL_Rect& panel);
    void draw_members(int x, int& y);
    void draw_pending(int x, int& y);
    int count_content_rows() const;
    int compute_panel_height() const;

 public:
    ClanPanel(SDL_Renderer* renderer, const std::string& font_path, int win_width, int win_height);
    ~ClanPanel();

    ClanPanel(const ClanPanel&) = delete;
    ClanPanel& operator=(const ClanPanel&) = delete;

    void set_data(const ClanReviewUpdate& update);
    void hide();
    bool is_visible() const;
    void draw();
};

#endif
