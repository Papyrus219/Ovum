#ifndef OVUM_SURVIVE_BEHAVIOR_MANAGER_HPP
#define OVUM_SURVIVE_BEHAVIOR_MANAGER_HPP

#include <Ovum/behaviors/entity_behavior_manager.hpp>
#include <Eruptor/scene/render_object.hpp>

namespace ovum
{

struct Entiety_data;

class Survive_behavior_manager : public ovum::Entity_behavior_manager
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
};

}

#endif // OVUM_SURVIVE_BEHAVIOR_MANAGER_HPP
