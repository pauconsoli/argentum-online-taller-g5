#ifndef CAMERA_H
#define CAMERA_H

class Camera {
 private:
    int x;
    int y;
    int screen_w;
    int screen_h;

 public:
    Camera(int screen_w, int screen_h);

    void center_on(int player_x, int player_y, int map_pixel_w, int map_pixel_h);

    int get_screen_x(int world_x) const;
    int get_screen_y(int world_y) const;

    int get_x() const;
    int get_y() const;
};

#endif
