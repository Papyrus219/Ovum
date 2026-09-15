#ifndef OVUM_SENSE_SIZE_SPEED_EVO_BEHAVIOR_MANAGER_HPP
#define OVUM_SENSE_SIZE_SPEED_EVO_BEHAVIOR_MANAGER_HPP

#include <Ovum/behaviors/entity_behavior_manager.hpp>
#include <Ovum/simulation_scene.hpp>
#include <Eruptor/scene/scene.hpp>

namespace ovum
{

class Sense_size_speed_evo_behavior_manager : public ovum::Entity_behavior_manager
{
public:
    virtual void Setup() override;

    virtual void Update_graph() override;

    virtual void New_day() override;

    virtual void Update_ai(float delta_time) override;

    virtual void React_to_event(const eruptor::event::Event& event) override;

private:
    void Update_hunting(eruptor::scene::Render_object & render_object, Entiety_data & entity_data, float delta_time);
    void Update_return(eruptor::scene::Render_object & render_object, Entiety_data & entity_data, float delta_time);

    std::unordered_map<float, uint32_t> entieties_speed{};
    std::uniform_real_distribution<float> size_evolution_distributor{-0.29, 0.29};
};

}

#endif // OVUM_SENSE_SIZE_SPEED_EVO_BEHAVIOR_MANAGER_HPP
