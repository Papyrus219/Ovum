#ifndef OVUM_SPEED_EVO_BEHAVIOR_MANAGER_HPP
#define OVUM_SPEED_EVO_BEHAVIOR_MANAGER_HPP

#include <Ovum/behaviors/entity_behavior_manager.hpp>
#include <Eruptor/scene/render_object.hpp>

namespace ovum
{

class Entiety_data;

class Speed_evo_behavior_manager : public ovum::Entity_behavior_manager
{
public:
    void Update_ai(float delta_time) override;
    void New_day() override;

private:
    void Update_hunting(eruptor::scene::Render_object & render_object, Entiety_data & entity_data, float delta_time);
    void Update_return(eruptor::scene::Render_object & render_object, Entiety_data & entity_data, float delta_time);
};

}

#endif // OVUM_SPEED_EVO_BEHAVIOR_MANAGER_HPP
