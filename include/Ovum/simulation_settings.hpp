#ifndef OVUM_SIMULATION_SETTINGS_HPP
#define OVUM_SIMULATION_SETTINGS_HPP

#include <Ovum/formula.hpp>
#include <cstdint>

namespace ovum
{

struct Simulation_settings
{
    bool is_speed_evo_enabled{};
    bool is_size_evo_enabled{};
    bool is_sense_evo_enabled{};

    uint32_t start_energy{};
    uint32_t food_per_day{};
    uint32_t food_survive_need{};
    uint32_t food_reproduction_need{};

    Formula energy_formula{};
    Registry energy_formula_registry{};
};

}

#endif //OVUM_SIMULATION_SETTINGS_HPP
