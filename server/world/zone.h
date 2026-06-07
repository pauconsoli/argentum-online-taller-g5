#ifndef ZONE_H
#define ZONE_H

#include <string>

class Zone {
 public:
    virtual ~Zone() = default;

    virtual bool is_safe() const = 0;

    virtual bool can_spawn() const = 0;

    virtual const std::string& get_name() const = 0;
};

#endif
