#include <Ovum/behaviors/entity_behavior_manager.hpp>
#include <Ovum/simulation_state.hpp>
#include <Ovum/app.hpp>

using namespace ovum;

ovum::Entity_behavior_manager::Entity_behavior_manager(Simulation_state & sim_state_): sim_state{sim_state_}
{
    main_scene = sim_state.main_scene;
    app = sim_state.app;

    this->generator = &sim_state.generator;
    this->decision_distributor = &sim_state.decision_distributor;
    this->evolution_distributor = &sim_state.evolution_distributor;
    this->rotation_distributor = &sim_state.rotation_distributor;
    this->x_pos_distribution = &sim_state.x_pos_distribution;
    this->z_pos_distribution = &sim_state.z_pos_distribution;
}

void ovum::Entity_behavior_manager::Spawn_food(uint32_t food_amount)
{
    float margin{0.6f};
    sim_state.x_pos_distribution.param( std::uniform_real_distribution<float>::param_type{app->world_min.x + margin, app->world_max.x - margin} );
    sim_state.z_pos_distribution.param( std::uniform_real_distribution<float>::param_type{app->world_min.z + margin, app->world_max.z - margin} );

    for(auto i{0UZ}; i < food_amount; i++)
    {
        auto id = main_scene->Add_food();
        auto render_id = main_scene->food[ id ].render_object_id;
        main_scene->render_objects[ render_id ].Set_position( {sim_state.x_pos_distribution(sim_state.random_device), 0.1f, sim_state.z_pos_distribution(sim_state.random_device)} );
    }
}
