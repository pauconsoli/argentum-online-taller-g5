#ifndef PLAYER_H
#define PLAYER_H

// Player: representa a un jugador en el juego, con atributos específicos como nombre, 
// raza, clase, oro, experiencia, etc. Hereda de Character

#include <cstdint>
#include <string>

#include "character.h"
#include "player_class.h"
#include "player_race.h"

class Player: public Character {
private:

    std::string name;

    PlayerRace p_race;
    PlayerClass p_class;

    uint64_t gold;
    uint64_t experience;

    bool meditating;

public:
    Player(
            uint32_t id,
            const std::string& name,
            PlayerRace p_race,
            PlayerClass p_class,
            int level,
            int max_hp,
            int max_mana,
            int strength,
            int agility,
            int intelligence,
            int constitution,
            const Position& position);

    ~Player() override = default;

    bool can_cast_magic() const override;

    void move(const Position& new_position);

    void add_gold(uint64_t amount);

    bool remove_gold(uint64_t amount);

    void add_experience(uint64_t amount);

    void start_meditating();

    void stop_meditating();

    bool is_meditating() const;

    const std::string& get_name() const;

    PlayerRace get_race() const;

    PlayerClass get_class() const;

    uint64_t get_gold() const;

    uint64_t get_experience() const;
};

#endif
