#include <Ovum/entity_behavior_manager.hpp>
#include <Ovum/simulation_state.hpp>
#include <Ovum/app.hpp>
#include <Ovum/gp_communicator.hpp>

using namespace ovum;

void ovum::Entity_behavior_manager::Init(Simulation_state & sim_state)
{
    this->sim_state = &sim_state;
    this->main_scene = this->sim_state->main_scene;
    this->app = this->sim_state->app;
    this->gp_comm_pop = &this->sim_state->gp_comm_pop;

    this->x_pos_distribution = &this->sim_state->x_pos_distribution;
    this->z_pos_distribution = &this->sim_state->z_pos_distribution;

    sim_settings = & sim_state.sim_settings;
}

void ovum::Entity_behavior_manager::Update_ai(float delta_time)
{
    for(auto & entity : main_scene->entieties)
    {
        switch(entity.ai_data.state)
        {
            case Ai_state::HUNTING:
                if(sim_settings->is_sense_used && entity.ai_data.desire_dirr != glm::vec3{})
                {
                    entity.Rotate_to_intrest(*main_scene, delta_time);
                }
                else
                {
                    Rotate_random(entity, delta_time);
                }

                Corect_wander_direction(entity);
                Walk_forward(entity, delta_time);

                break;
            case Ai_state::RETURN:
                Rotate_to_closest_border(entity, delta_time);
                Walk_forward(entity, delta_time);
                break;
            default:
                break;
        }

        State_change_check(entity);
    }

    if(finished_entities >= main_scene->entieties.size())
    {
        New_day();
    }
}

void ovum::Entity_behavior_manager::New_day()
{
    sim_state->day_passed++;

    for(auto id : dead_ids)
    {
        main_scene->Remove_entity( id );
    }
    dead_ids.clear();

    while(!main_scene->food.empty())
    {
        main_scene->Remove_food( main_scene->food.back().render_object_id );
    }

    auto & entieties = main_scene->entieties;

    for(auto i{0UZ}; i < entieties.size(); i++)
    {
        if(entieties[i].ai_data.state != Ai_state::RESTING)
        {
            continue;
        }

        entieties[i].energy = sim_settings->start_energy;
        entieties[i].ai_data.state = Ai_state::HUNTING;
        entieties[i].ai_data.time_elapsed = 0;

        if(entieties[i].food_eaten >= sim_settings->food_reproduction_need)
        {
            auto new_id = main_scene->Add_entity();
            Inheritate_and_evolve_stats(entieties[i], entieties[new_id]);
        }

        entieties[i].food_eaten = 0;
    }

    finished_entities = 0;
    Spawn_food( sim_settings->food_per_day );

    sim_state->Update_graph();
    Update_graph();
}

void ovum::Entity_behavior_manager::State_change_check(Entiety_data& entity_data)
{
    auto & render_object = main_scene->render_objects[ entity_data.render_object_id ];
    auto pos = render_object.Get_position();

    if(entity_data.ai_data.state == Ai_state::HUNTING)
    {
        if((entity_data.food_eaten >= sim_settings->food_reproduction_need) || (entity_data.energy < 5 && entity_data.food_eaten >= sim_settings->food_survive_need))
        {
            entity_data.ai_data.state = Ai_state::RETURN;
        }
        else if(entity_data.energy <= 0)
        {
            render_object.color = eruptor::resource::Color{255, 255, 255, 255};
            entity_data.ai_data.state = Ai_state::DEAD;
            dead_ids.push_back( entity_data.render_object_id );

            finished_entities++;
        }
    }
    else if(entity_data.ai_data.state == Ai_state::RETURN)
    {
        float dist_max_x = app->world_max.x - pos.x;
        float dist_min_x = pos.x - app->world_min.x;
        float dist_max_z = app->world_max.z - pos.z;
        float dist_min_z = pos.z - app->world_min.z;

        float min_dist = std::min({dist_max_x, dist_min_x, dist_max_z, dist_min_z});

        float wall_margin{0.3f};

        if(min_dist <= wall_margin)
        {
            entity_data.ai_data.state = Ai_state::RESTING;

            if(min_dist == dist_max_x)
            {
                render_object.Set_position( {app->world_max.x, pos.y, pos.z} );
            }
            else if(min_dist == dist_max_z)
            {
                render_object.Set_position( {pos.x, pos.y, app->world_max.z} );
            }
            else if(min_dist == dist_min_x)
            {
                render_object.Set_position( {app->world_min.x, pos.y, pos.z} );
            }
            else if(min_dist == dist_min_z)
            {
                render_object.Set_position( {pos.x, pos.y, app->world_min.z} );
            }

            finished_entities++;
        }
        else if(entity_data.energy <= 0)
        {
            render_object.color = eruptor::resource::Color{255, 255, 255, 255};
            entity_data.ai_data.state = Ai_state::DEAD;
            dead_ids.push_back( entity_data.render_object_id );

            finished_entities++;
        }
    }
}

void ovum::Entity_behavior_manager::Inheritate_and_evolve_stats(Entiety_data & parent, Entiety_data & child)
{
    auto & render_objects = main_scene->render_objects;

    child.speed = parent.speed;
    child.size = parent.size;
    child.sense = parent.sense;
    child.ai_data = parent.ai_data;

    if(sim_settings->is_speed_evo_enabled)
    {
        child.speed += sim_settings->speed_evo_distributor( sim_settings->generator );

        if(child.speed < 0.1)
        {
            child.speed = 0.1;
        }
    }
    if(sim_settings->is_size_evo_enabled)
    {
        child.size += sim_settings->size_evo_distributor( sim_settings->generator );

        if(child.size < 0.1)
        {
            child.size = 0.1;
        }
    }
    if(sim_settings->is_sense_evo_enabled)
    {
        child.sense += sim_settings->sense_evo_distributor( sim_settings->generator );

        if(child.sense < 0.1)
        {
            child.sense = 0.1;
        }
    }

    child.ai_data.state = Ai_state::HUNTING;
    child.energy = sim_settings->start_energy;

    render_objects[ child.render_object_id ] = render_objects[ parent.render_object_id ];
    render_objects[ child.render_object_id ].Set_scale( {(2 * child.size) + 4, (2 * child.size) + 4, (2 * child.size) + 4} );

    if(sim_settings->is_sense_used)
    {
        if(auto sense_hitbox_wraper = app->physic_manager->Get_hitbox_data(1, child.render_object_id))
        {
            auto & sense_hitbox = sense_hitbox_wraper.value().get();

            auto orginal_scale = render_objects[ child.render_object_id ].Get_scale();
            auto new_sense = child.sense;
            sense_hitbox.Set_individual_scale( {orginal_scale.x + new_sense, orginal_scale.y + new_sense, orginal_scale.z + new_sense } );
        }
    }
}

void ovum::Entity_behavior_manager::Rotate_random(Entiety_data& entity_data, float delta_time)
{
    auto & render_object = main_scene->render_objects[ entity_data.render_object_id ];

    if(entity_data.ai_data.is_desire_rot)
    {
        entity_data.ai_data.time_elapsed += delta_time;

        if(entity_data.ai_data.time_elapsed >= 0.5)
        {
            if((sim_settings->decision_distributor)(sim_settings->generator) == 1)
            {
                float angle = sim_settings->rotation_distributor(sim_settings->generator);
                if(sim_settings->decision_distributor(sim_settings->generator) == 1)
                {
                    angle = -angle;
                }

                entity_data.ai_data.is_desire_rot = false;
                entity_data.ai_data.desire_y_rot = sim_state->Normilize_angle(entity_data.ai_data.desire_y_rot + angle);
            }

            entity_data.ai_data.time_elapsed = 0;
        }
    }
    else
    {
        float diff = sim_state->Normilize_angle( entity_data.ai_data.desire_y_rot - entity_data.ai_data.curr_y_rot );
        if(std::abs(diff) > 0.05f)
        {
            float step = std::copysign( std::max(1.0f, entity_data.speed) * delta_time, diff);
            if(std::abs(step) > std::abs(diff))
            {
                step = diff;
            }

            render_object.Rotate({0.0f, step, 0.0f});
            entity_data.ai_data.curr_y_rot = sim_state->Normilize_angle(entity_data.ai_data.curr_y_rot + step);
        }
        else
        {
            entity_data.ai_data.is_desire_rot = true;
        }
    }
}

void ovum::Entity_behavior_manager::Rotate_to_closest_border(Entiety_data& entity_data, float delta_time)
{
    auto & render_object = main_scene->render_objects[ entity_data.render_object_id ];
    glm::vec3 pos = render_object.Get_position();

    float dist_max_x = app->world_max.x - pos.x;
    float dist_min_x = pos.x - app->world_min.x;
    float dist_max_z = app->world_max.z - pos.z;
    float dist_min_z = pos.z - app->world_min.z;

    float min_dist = std::min({dist_max_x, dist_min_x, dist_max_z, dist_min_z});

    glm::vec3 target_dir{0.0f};

    if(min_dist == dist_max_x)
    {
        target_dir = {1.0f, 0.0f, 0.0f};
    }
    else if(min_dist == dist_min_x)
    {
        target_dir = {-1.0f, 0.0f, 0.0f};
    }
    else if(min_dist == dist_max_z)
    {
        target_dir = {0.0f, 0.0f, 1.0f};
    }
    else
    {
        target_dir = {0.0f, 0.0f, -1.0f};
    }

    float desired_y_rot = std::atan2(-target_dir.z, target_dir.x);
    entity_data.ai_data.desire_y_rot = desired_y_rot;

    float diff = sim_state->Normilize_angle(entity_data.ai_data.desire_y_rot - entity_data.ai_data.curr_y_rot);

    if(std::abs(diff) > 0.05f)
    {
        float step = std::copysign(entity_data.speed * delta_time, diff);

        if(std::abs(step) > std::abs(diff))
        {
            step = diff;
        }

        render_object.Rotate({0.0f, step, 0.0f});
        entity_data.ai_data.curr_y_rot = sim_state->Normilize_angle(entity_data.ai_data.curr_y_rot + step);
    }
}

void ovum::Entity_behavior_manager::Corect_wander_direction(Entiety_data & entity_data)
{
    auto & render_object = main_scene->render_objects[ entity_data.render_object_id ];
    auto pos = render_object.Get_position();
    auto forward  = render_object.Get_rotation() * glm::vec3{1.0f, 0.0f, 0.0f} ;

    bool near_wall{};

    if(pos.x > app->world_max.x - sim_state->wall_margin)
    {
        near_wall = true;
    }
    else if(pos.x < app->world_min.x + sim_state->wall_margin)
    {
        near_wall = true;
    }

    if(pos.z > app->world_max.z - sim_state->wall_margin)
    {
        near_wall = true;
    }
    else if(pos.z < app->world_min.z + sim_state->wall_margin)
    {
        near_wall = true;
    }

    if(near_wall)
    {
        glm::vec3 to_center{(app->world_min.x + app->world_max.x) * 0.5f - pos.x, 0.0f,(app->world_min.z + app->world_max.z) * 0.5f - pos.z };

        to_center = glm::normalize(to_center);

        entity_data.ai_data.desire_y_rot = std::atan2(-to_center.z, to_center.x);
        entity_data.ai_data.is_desire_rot = false;
    }

    bool hit_wall = false;

    if(pos.x > app->world_max.x) {pos.x = app->world_max.x; hit_wall = true;}
    else if(pos.x < app->world_min.x) {pos.x = app->world_min.x; hit_wall = true;}

    if(pos.z > app->world_max.z) {pos.z = app->world_max.z; hit_wall = true;}
    else if(pos.z < app->world_min.z) {pos.z = app->world_min.z; hit_wall = true;}

    if(hit_wall)
    {
        render_object.Set_position(pos);

        if(pos.x == app->world_max.x || pos.x == app->world_min.x) forward.x = -forward.x;
        if(pos.z == app->world_max.z || pos.z == app->world_min.z) forward.z = -forward.z;

        float new_y_rot = std::atan2(-forward.z, forward.x);

        entity_data.ai_data.curr_y_rot = new_y_rot;
        entity_data.ai_data.desire_y_rot = new_y_rot;
        entity_data.ai_data.is_desire_rot = false;

        render_object.Set_rotation_quad( glm::angleAxis(new_y_rot, glm::vec3{0.0f, 1.0f, 0.0f}) );
    }
}

void ovum::Entity_behavior_manager::Walk_forward(Entiety_data & entity_data, float delta_time)
{
    auto & render_object = main_scene->render_objects[ entity_data.render_object_id ];
    glm::vec3 forward  = render_object.Get_rotation() * glm::vec3{1.0f, 0.0f, 0.0f} ;

    render_object.Move( forward * entity_data.speed * delta_time );
    entity_data.energy -= sim_state->formula.Evaluate(entity_data) * delta_time;
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
        main_scene->render_objects[ render_id ].Set_position( {sim_state->x_pos_distribution(sim_settings->generator), 0.1f, sim_state->z_pos_distribution(sim_settings->generator)} );
    }
}

void ovum::Entity_behavior_manager::Update_graph()
{

}

void ovum::Entity_behavior_manager::React_to_event(const eruptor::event::Event& event)
{
    if(auto colision = event.Get_if<eruptor::event::Event::Collision_occurred>())
    {
        auto entity_a_wraper = main_scene->Get_if_is_entiety( colision->object_a_id );
        auto entity_b_wraper = main_scene->Get_if_is_entiety( colision->object_b_id );
        auto food_a_wraper = main_scene->Get_if_is_food( colision->object_a_id );
        auto food_b_wraper = main_scene->Get_if_is_food( colision->object_b_id );

        if( (colision->object_a_layer | colision->object_b_layer) == 0 )
        {
            if( entity_a_wraper && food_b_wraper )
            {
                auto & entity_a = entity_a_wraper.value().get();
                if(sim_settings->is_hunger_can_be_satisfied && entity_a.food_eaten >= sim_settings->food_reproduction_need)
                {
                    return;
                }

                entity_a.Eat();
                main_scene->Remove_food( food_b_wraper.value().get().render_object_id );
            }
            else if( entity_b_wraper && food_a_wraper )
            {
                auto & entity_b = entity_b_wraper.value().get();
                if(sim_settings->is_hunger_can_be_satisfied && entity_b.food_eaten >= sim_settings->food_reproduction_need)
                {
                    return;
                }

                entity_b.Eat();
                main_scene->Remove_food( food_a_wraper.value().get().render_object_id );
            }
            else if( sim_settings->is_canibalism_enabled )
            {
                if(entity_a_wraper && entity_b_wraper)
                {
                    auto & entity_a = entity_a_wraper.value().get();
                    auto & entity_b = entity_b_wraper.value().get();

                    if(entity_a.size >= (1.30 * entity_b.size) && entity_a.ai_data.state == Ai_state::HUNTING && entity_b.ai_data.state == Ai_state::HUNTING)
                    {
                        if(sim_settings->is_hunger_can_be_satisfied && entity_a.food_eaten >= sim_settings->food_reproduction_need)
                        {
                            return;
                        }

                        entity_a.Eat(sim_settings->food_reproduction_need);

                        main_scene->Remove_entity( entity_b.render_object_id );
                    }
                    else if(entity_b.size >= (1.30 * entity_a.size) && entity_a.ai_data.state == Ai_state::HUNTING && entity_b.ai_data.state == Ai_state::HUNTING)
                    {
                        if(sim_settings->is_hunger_can_be_satisfied && entity_b.food_eaten >= sim_settings->food_reproduction_need)
                        {
                            return;
                        }

                        entity_b.Eat(sim_settings->food_reproduction_need);

                        main_scene->Remove_entity( entity_a.render_object_id );
                    }
                }
            }
        }
        else if(sim_settings->is_sense_used)
        {
            if( (colision->object_a_layer | colision->object_b_layer) == 1 && (colision->object_a_layer & colision->object_b_layer) == 0 )
            {
                if(entity_a_wraper)
                {
                    auto & entity_a = entity_a_wraper.value().get();
                    if(entity_a.ai_data.state != Ai_state::HUNTING) return;

                    if(food_b_wraper)
                    {
                        entity_a.ai_data.Add_dirr_of_intrest(main_scene->render_objects[colision->object_a_id].Get_position(), main_scene->render_objects[colision->object_b_id].Get_position(), 1, 2);
                    }
                    else if(entity_b_wraper)
                    {
                        auto & entity_b = entity_b_wraper.value().get();

                        if(entity_b.size <= (1.30 * entity_a.size))
                        {
                            entity_a.ai_data.Add_dirr_of_intrest(main_scene->render_objects[colision->object_a_id].Get_position(), main_scene->render_objects[colision->object_b_id].Get_position(), 1, 2);
                        }
                        else if(entity_b.size >= (1.30 * entity_a.size))
                        {
                            entity_a.ai_data.Add_dirr_of_intrest(main_scene->render_objects[colision->object_b_id].Get_position(), main_scene->render_objects[colision->object_a_id].Get_position(), 100, 2);
                        }
                    }
                }
            }
            else if(colision->object_b_layer == 1)
            {
                if(entity_b_wraper)
                {
                    auto & entity_b = entity_b_wraper.value().get();
                    if(entity_b.ai_data.state != Ai_state::HUNTING) return;

                    if(food_a_wraper)
                    {
                        entity_b.ai_data.Add_dirr_of_intrest(main_scene->render_objects[colision->object_b_id].Get_position(), main_scene->render_objects[colision->object_a_id].Get_position(), 1, 2);
                    }
                    else if(entity_a_wraper)
                    {
                        auto & entity_a = entity_a_wraper.value().get();

                        if(entity_a.size <= (1.30 * entity_b.size))
                        {
                            entity_b.ai_data.Add_dirr_of_intrest(main_scene->render_objects[colision->object_b_id].Get_position(), main_scene->render_objects[colision->object_a_id].Get_position(), 1, 2);
                        }
                        else if(entity_a.size >= (1.30 * entity_b.size))
                        {
                            entity_b.ai_data.Add_dirr_of_intrest(main_scene->render_objects[colision->object_a_id].Get_position(), main_scene->render_objects[colision->object_b_id].Get_position(), 100, 2);
                        }
                    }
                }
            }
        }
    }
}
