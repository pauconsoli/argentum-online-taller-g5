#ifndef CITY_H
#define CITY_H

// preguntar, ciudades Y pueblos son lo mismo?
// o son dos tipos de zonas distintas?
// por ahora asumo que son lo mismo

// EN EL ENUNCIADO: Ciudades y pueblos
// "A lo largo de las tierras de Argentum hay pequeñas ciudades y pueblos en las que
// los jugadores podrán encontrar al menos a un sacerdote, un comerciante y un banquero"

#include "zone.h"

class City: public Zone {
 public:
    bool is_safe() const override;

    bool can_spawn() const override;
};

#endif
