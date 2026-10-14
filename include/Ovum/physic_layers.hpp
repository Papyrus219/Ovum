#ifndef OVUM_SIMULATION_STATE_CPP
#define OVUM_SIMULATION_STATE_CPP

#include <cstdint>

namespace ovum
{

enum Physic_layer: uint8_t
{
    ENTITY = 0,
    SENSE = 1,
    FOOD = 2
};

}

#endif //OVUM_SIMULATION_STATE_CPP
