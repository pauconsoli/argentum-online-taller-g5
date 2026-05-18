#ifndef POSITION_H
#define POSITION_H

struct Position {
    int x;
    int y;

    bool operator<(const Position& other) const {
        if (x != other.x) return x < other.x;
        return y < other.y;
    }

};

//por ahora, solo una struct, si después necesitamos métodos la convertimos a clase

#endif