#ifndef OVUM_SIMULATION_SETTINGS_HPP
#define OVUM_SIMULATION_SETTINGS_HPP

#include <Ovum/formula.hpp>
#include <glm/glm.hpp>
#include <cstdint>
#include <random>

namespace ovum
{

struct Simulation_settings
{
    bool is_speed_evo_enabled{};
    bool is_size_evo_enabled{};
    bool is_sense_evo_enabled{};

    bool is_sense_used{};

    bool is_canibalism_enabled{};

    uint32_t start_energy{};
    uint32_t food_per_day{};
    uint32_t food_survive_need{};
    uint32_t food_reproduction_need{};

    std::mt19937  generator{};
    std::uniform_int_distribution<uint8_t> decision_distributor{0, 2};
    std::uniform_real_distribution<float> speed_evo_distributor{};
    std::uniform_real_distribution<float> size_evo_distributor{};
    std::uniform_real_distribution<float> sense_evo_distributor{};
    std::uniform_real_distribution<float> rotation_distributor{ 0, glm::half_pi<float>() };

    Formula energy_formula{};
    Registry energy_formula_registry{};
};

}

#endif //OVUM_SIMULATION_SETTINGS_HPP
