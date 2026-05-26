#ifndef SPELL_H
#define SPELL_H

#include <string>

class Spell {
 private:
    std::string name;

    int min_damage;
    int max_damage;

    int min_heal;
    int max_heal;

 public:
    Spell(const std::string& name, int min_damage, int max_damage, int min_heal, int max_heal);

    ~Spell();

    std::string get_name() const;

    int get_min_damage() const;
    int get_max_damage() const;

    int get_min_heal() const;
    int get_max_heal() const;

    bool does_damage() const;
    bool does_heal() const;
};

#endif
