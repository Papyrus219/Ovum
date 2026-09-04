#ifndef OVUM_ENTITY_BEHAVIOR_MANAGER_HPP
#define OVUM_ENTITY_BEHAVIOR_MANAGER_HPP

#include <Eruptor/event/event.hpp>
#include <cstdint>
#include <random>

namespace ovum
{

class Simulation_state;
struct Simulation_scene;
class App;
class GP_communicator;

class Entity_behavior_manager
{
public:
    void Init(Simulation_state & sim_state);

    virtual void Setup() = 0;
    virtual void Update_ai(float delta_time) = 0;
    virtual void Spawn_food(uint32_t food_amount);
    virtual void New_day() = 0;

    virtual void Update_graph() = 0;

    virtual void React_to_event(const eruptor::event::Event & event) = 0;

protected:
    bool day_should_end{};
    size_t finished_entities{};
    std::vector<uint32_t> dead_ids{};

    std::mt19937 * generator{};
    std::uniform_int_distribution<uint8_t> * decision_distributor{};
    std::uniform_real_distribution<float> * evolution_distributor{};
    std::uniform_real_distribution<float> * rotation_distributor{};
    std::uniform_real_distribution<float> * x_pos_distribution{};
    std::uniform_real_distribution<float> * z_pos_distribution{};

    Simulation_state * sim_state{};
    ovum::GP_communicator * gp_comm{};

    ovum::Simulation_scene * main_scene{};
    ovum::App * app{};
};

}

#endif // OVUM_ENTITY_BEHAVIOR_MANAGER_HPP
