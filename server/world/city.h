#ifndef CITY_H
#define CITY_H

#include <string>

#include "zone.h"

class City: public Zone {
 private:
    std::string name;

 public:
    explicit City(std::string name = "Ciudad");

    bool is_safe() const override;
    bool can_spawn() const override;

    const std::string& get_name() const override;
};

#endif
