#ifndef OVUM_ENTITY_BEHAVIOR_MANAGER_HPP
#define OVUM_ENTITY_BEHAVIOR_MANAGER_HPP

#include <Eruptor/event/event.hpp>
#include <Ovum/gp_communicator.hpp>
#include <Ovum/simulation_settings.hpp>
#include <Ovum/simulation_scene.hpp>
#include <cstdint>
#include <random>

namespace ovum
{

class Simulation_state;
struct Simulation_scene;
class App;

class Entity_behavior_manager
{
public:
    void Init(Simulation_state & sim_state);

    void Setup();
    void Update_ai(float delta_time);
    void Spawn_food(uint32_t food_amount);
    void New_day();

    void Update_graph();

    void React_to_event(const eruptor::event::Event & event);

protected:
    void State_change_check(Entiety_data & entity_data);

    void Inheritate_and_evolve_stats(Entiety_data & parent, Entiety_data & child);

    void Rotate_random(Entiety_data & entity_data, float delta_time);
    void Rotate_to_closest_border(Entiety_data & entity_data, float delta_time);

    void Corect_wander_direction(Entiety_data & entity_data);

    void Walk_forward(Entiety_data & entity_data, float delta_time);


    bool day_should_end{};
    size_t finished_entities{};
    std::vector<uint32_t> dead_ids{};

    std::uniform_real_distribution<float> * x_pos_distribution{};
    std::uniform_real_distribution<float> * z_pos_distribution{};

    Simulation_settings * sim_settings{};

    Simulation_state * sim_state{};
    ovum::GP_communicator * gp_comm_pop{};
    ovum::GP_communicator gp_comm_stats{};

    ovum::Simulation_scene * main_scene{};
    ovum::App * app{};
};

}

#endif // OVUM_ENTITY_BEHAVIOR_MANAGER_HPP
