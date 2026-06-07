#include "camera.h"

Camera::Camera(int screen_w, int screen_h): x(0), y(0), screen_w(screen_w), screen_h(screen_h) {}

void Camera::center_on(int player_x, int player_y, int map_pixel_w, int map_pixel_h) {
    x = player_x - screen_w / 2;
    y = player_y - screen_h / 2;

    int max_x = map_pixel_w - screen_w;
    int max_y = map_pixel_h - screen_h;
    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
    if (x > max_x)
        x = max_x > 0 ? max_x : 0;
    if (y > max_y)
        y = max_y > 0 ? max_y : 0;
}

int Camera::get_screen_x(int world_x) const {
    return world_x - x;
}

int Camera::get_screen_y(int world_y) const {
    return world_y - y;
}

int Camera::get_x() const {
    return x;
}
int Camera::get_y() const {
    return y;
}
