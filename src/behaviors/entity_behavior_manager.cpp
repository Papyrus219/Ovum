#include <Ovum/behaviors/entity_behavior_manager.hpp>
#include <Ovum/simulation_state.hpp>
#include <Ovum/app.hpp>
#include <Ovum/gp_communicator.hpp>

using namespace ovum;

void ovum::Entity_behavior_manager::Init(Simulation_state & sim_state)
{
    this->sim_state = &sim_state;
    this->main_scene = this->sim_state->main_scene;
    this->app = this->sim_state->app;
    this->gp_comm = &this->sim_state->gp_comm;

    this->generator = &this->sim_state->generator;
    this->decision_distributor = &this->sim_state->decision_distributor;
    this->evolution_distributor = &this->sim_state->evolution_distributor;
    this->rotation_distributor = &this->sim_state->rotation_distributor;
    this->x_pos_distribution = &this->sim_state->x_pos_distribution;
    this->z_pos_distribution = &this->sim_state->z_pos_distribution;
}

void ovum::Entity_behavior_manager::Spawn_food(uint32_t food_amount)
{
    float margin{0.6f};
    sim_state->x_pos_distribution.param( std::uniform_real_distribution<float>::param_type{app->world_min.x + margin, app->world_max.x - margin} );
    sim_state->z_pos_distribution.param( std::uniform_real_distribution<float>::param_type{app->world_min.z + margin, app->world_max.z - margin} );

    for(auto i{0UZ}; i < food_amount; i++)
    {
        auto id = main_scene->Add_food();
        auto render_id = main_scene->food[ id ].render_object_id;
        main_scene->render_objects[ render_id ].Set_position( {sim_state->x_pos_distribution(sim_state->random_device), 0.1f, sim_state->z_pos_distribution(sim_state->random_device)} );
    }
}
