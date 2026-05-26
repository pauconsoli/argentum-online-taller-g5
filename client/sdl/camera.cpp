#include "camera.h"

Camera::Camera(int screen_w, int screen_h) : 
    x(0), y(0), screen_w(screen_w), screen_h(screen_h) {}

void Camera::center_on(int player_x, int player_y) {
    x = player_x - screen_w / 2;
    y = player_y - screen_h / 2;
}

int Camera::get_screen_x(int world_x) const {
    return world_x - x;
}

int Camera::get_screen_y(int world_y) const {
    return world_y - y;
}