#ifndef CITY_H
#define CITY_H

#include <string>

#include "common/position.h"
#include "zone.h"

class City: public Zone {
 private:
    std::string name;
    Position priest_position;
    Position merchant_position;
    Position banker_position;

 public:
    explicit City(std::string name = "Ciudad", Position priest_position = {0, 0},
                  Position merchant_position = {0, 0}, Position banker_position = {0, 0});

    bool is_safe() const override;
    bool can_spawn() const override;

    const std::string& get_name() const override;
    ZoneType get_type() const override;
    Position get_priest_position() const;
    Position get_merchant_position() const;
    Position get_banker_position() const;
};

#endif
