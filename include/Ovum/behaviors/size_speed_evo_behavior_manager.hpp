#ifndef OVUM_SIZE_SPEED_EVO_BEHAVIOR_MANAGER_HPP
#define OVUM_SIZE_SPEED_EVO_BEHAVIOR_MANAGER_HPP

#include <Ovum/behaviors/entity_behavior_manager.hpp>
#include <Ovum/simulation_scene.hpp>
#include <Eruptor/scene/scene.hpp>

namespace ovum
{

class Size_speed_evo_behavior_manager : public ovum::Entity_behavior_manager
{
public:
    void Setup() override;

    void Update_graph() override;
    void New_day() override;
    void Update_ai(float delta_time) override;

    void React_to_event(const eruptor::event::Event & event) override;

private:
    void Update_hunting(eruptor::scene::Render_object & render_object, Entiety_data & entity_data, float delta_time);
    void Update_return(eruptor::scene::Render_object & render_object, Entiety_data & entity_data, float delta_time);

    std::unordered_map<float, uint32_t> entieties_speed{};
};

}

#endif // OVUM_SIZE_SPEED_EVO_BEHAVIOR_MANAGER_HPP
