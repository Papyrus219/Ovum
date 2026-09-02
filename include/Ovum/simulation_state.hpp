#ifndef OVUM_SIMULATION_STATE_HPP
#define OVUM_SIMULATION_STATE_HPP

#include <Ovum/app_state.hpp>
#include <Ovum/simulation_scene.hpp>
#include <Ovum/gp_communicator.hpp>
#include <random>

namespace ovum
{

class Simulation_state : public ovum::App_state
{
public:
    virtual void Init(App & app) override;
    virtual void Enter_state() override;

    virtual void Update() override;
    virtual void Render() override;
    virtual void React_to_event(const eruptor::event::Event & event) override;

    virtual std::string_view Get_state_name() override {return "Simulation";}

private:
    void New_day();

    void Update_ai(float delta_time);

    void Update_hunting(eruptor::scene::Render_object & render_object, Entiety_data & entity_data, float delta_time);
    void Update_return(eruptor::scene::Render_object & render_object, Entiety_data & entity_data, float delta_time);

    void Spawn_food(uint32_t food_amount);

    float Normilize_angle(float angle);

    ovum::Simulation_scene * main_scene{};

    std::random_device random_device{};
    std::mt19937 generator{ random_device() };
    std::uniform_int_distribution<uint8_t> decision_distributor{0, 1};
    std::uniform_real_distribution<float> evolution_distributor{-0.5, 0.5};
    std::uniform_real_distribution<float> rotation_distributor{-glm::half_pi<float>(), glm::half_pi<float>()};
    std::uniform_real_distribution<float> x_pos_distribution{};
    std::uniform_real_distribution<float> z_pos_distribution{};

    float time_acumulator{};

    GP_communicator gp_comm{};
    std::chrono::high_resolution_clock::time_point last_time{};

    std::unordered_map<float, uint32_t> entieties_speed{};

    float simulation_speed{15};
    bool day_should_end{};

    size_t finished_entities{};
    float wall_margin{3.0f};
};

}

#endif // OVUM_SIMULATION_STATE_HPP
