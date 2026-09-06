#include <Ovum/behaviors/size_speed_evo_behavior_manager.hpp>
#include <Ovum/simulation_state.hpp>
#include <Ovum/simulation_scene.hpp>
#include <Ovum/app.hpp>
#include <Ovum/gp_communicator.hpp>

using namespace ovum;

void ovum::Size_speed_evo_behavior_manager::Setup()
{
   // gp_comm->Enable_3d_points("Stats");
    gp_comm->Enable_2d_bars("SPEED");
    gp_comm->Set_x_axis_title("Speed");
    gp_comm->Set_y_axis_title("Entities count");
    //gp_comm->Set_z_axis_title("Size");
    gp_comm->Set_x_axis_range(0.0, 30.0);
    gp_comm->Set_y_axis_range(0, 10);
}

void ovum::Size_speed_evo_behavior_manager::Update_ai(float delta_time)
{
    for(auto & entity : main_scene->entieties)
    {
        auto & render_object = main_scene->render_objects[ entity.render_object_id ];

        switch(entity.ai_data.state)
        {
            case Ai_state::HUNTING:
                Update_hunting(render_object, entity, delta_time);
                break;
            case Ai_state::RETURN:
                Update_return(render_object, entity, delta_time);
                break;
            default:
                break;
        }
    }

    if(day_should_end)
    {
        New_day();
        day_should_end = false;
    }
}

void ovum::Size_speed_evo_behavior_manager::New_day()
{
    finished_entities = 0;

    while(!main_scene->food.empty())
    {
        main_scene->Remove_food( main_scene->food.back().render_object_id );
    }

    Spawn_food(100);

    auto & entieties = main_scene->entieties;
    auto & render_objects = main_scene->render_objects;

    for(auto id : dead_ids)
    {
        main_scene->Remove_entity( id );
    }
    dead_ids.clear();

    for(auto i{0UZ}; i < entieties.size(); i++)
    {
        if(entieties[i].ai_data.state == Ai_state::RESTING)
        {
            entieties[i].energy = 40;
            entieties[i].ai_data.state = Ai_state::HUNTING;
            entieties[i].ai_data.time_elapsed = 0;

            if(entieties[i].food_eaten >= 2)
            {
                auto new_id = main_scene->Add_entity();
                entieties[new_id].speed = entieties[i].speed;
                entieties[new_id].speed += (*evolution_distributor)(*generator);
                entieties[new_id].size = entieties[i].size;
                entieties[new_id].size += (*evolution_distributor)(*generator);
                entieties[new_id].ai_data = entieties[i].ai_data;
                entieties[new_id].ai_data.state = Ai_state::HUNTING;
                entieties[new_id].energy = 40;

                if(entieties[new_id].speed < 0.1) entieties[new_id].speed = 0.1;
                if(entieties[new_id].size < 0.1) entieties[new_id].size = 0.1;

                render_objects[ entieties[new_id].render_object_id ] = render_objects[ entieties[i].render_object_id ];
                render_objects[ entieties[new_id].render_object_id ].Set_scale( {entieties[new_id].size + 5, entieties[new_id].size + 5, entieties[new_id].size + 5} );
            }
            entieties[i].food_eaten = 0;
        }
    }
}

void ovum::Size_speed_evo_behavior_manager::Update_hunting(eruptor::scene::Render_object & render_object, Entiety_data & entity_data, float delta_time)
{
    glm::vec3 pos = render_object.Get_position();

    bool near_wall{};
    glm::vec3 desired_dir = render_object.Get_rotaion() * glm::vec3{1.0f, 0.0f, 0.0f};

    if(pos.x > app->world_max.x - sim_state->wall_margin)
    {
        desired_dir.x = -std::abs(desired_dir.x);
        near_wall = true;
    }
    else if(pos.x < app->world_min.x + sim_state->wall_margin)
    {
        desired_dir.x = std::abs(desired_dir.x);
        near_wall = true;
    }

    if(pos.z > app->world_max.z - sim_state->wall_margin)
    {
        desired_dir.z = -std::abs(desired_dir.z);
        near_wall = true;
    }
    else if(pos.z < app->world_min.z + sim_state->wall_margin)
    {
        desired_dir.z = std::abs(desired_dir.z);
        near_wall = true;
    }

    if(near_wall)
    {
        entity_data.ai_data.desire_y_rot = std::atan2(-desired_dir.z, desired_dir.x);
        entity_data.ai_data.is_desire_rot = false;
    }

    if(entity_data.ai_data.is_desire_rot)
    {
        entity_data.ai_data.time_elapsed += delta_time;

        entity_data.ai_data.is_desire_rot = false;
        entity_data.ai_data.desire_y_rot += (*rotation_distributor)(*generator);
        entity_data.ai_data.desire_y_rot = sim_state->Normilize_angle(entity_data.ai_data.desire_y_rot);;

        entity_data.ai_data.time_elapsed = 0;
    }
    else
    {
        float diff = sim_state->Normilize_angle( entity_data.ai_data.desire_y_rot - entity_data.ai_data.curr_y_rot );
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
        else
        {
            entity_data.ai_data.is_desire_rot = true;
        }
    }

    glm::vec3 forward  = render_object.Get_rotaion() * glm::vec3{1.0f, 0.0f, 0.0f} ;
    render_object.Move( forward * entity_data.speed * delta_time );
    entity_data.energy -= (entity_data.size * entity_data.size * entity_data.size) * (entity_data.speed * entity_data.speed) * delta_time;

    if(entity_data.energy < 5 && entity_data.food_eaten > 0)
    {
        entity_data.ai_data.state = Ai_state::RETURN;
    }
    else if(entity_data.energy <= 0)
    {
        render_object.color = eruptor::resource::Color{255, 255, 255, 255};
        entity_data.ai_data.state = Ai_state::DEAD;
        dead_ids.push_back( entity_data.render_object_id );

        finished_entities++;
        if(finished_entities >= main_scene->entieties.size())
        {
            day_should_end = true;
        }
    }

    pos = render_object.Get_position();
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

void ovum::Size_speed_evo_behavior_manager::Update_return(eruptor::scene::Render_object& render_object, Entiety_data& entity_data, float delta_time)
{
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

    glm::vec3 forward = render_object.Get_rotaion() * glm::vec3{1.0f, 0.0f, 0.0f};
    render_object.Move( forward * entity_data.speed * delta_time );
    entity_data.energy -= (entity_data.size * entity_data.size * entity_data.size) * (entity_data.speed * entity_data.speed) * delta_time;

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
        if(finished_entities >= main_scene->entieties.size())
        {
            day_should_end = true;
        }
    }
    else if(entity_data.energy <= 0)
    {
        render_object.color = eruptor::resource::Color{255, 255, 255, 255};
        entity_data.ai_data.state = Ai_state::DEAD;
        dead_ids.push_back( entity_data.render_object_id );

        finished_entities++;
        if(finished_entities >= main_scene->entieties.size())
        {
            day_should_end = true;
        }
    }
}

void ovum::Size_speed_evo_behavior_manager::Update_graph()
{
    gp_comm->Begin_frame();

    entieties_speed.clear();

    for(auto & entity : main_scene->entieties)
    {
        float bucket = std::round(entity.speed * 10.0f) / 10.0f;
        entieties_speed[bucket]++;
    }

    for(auto [speed, amount] : entieties_speed)
    {
        gp_comm->Stage_data({speed, amount});
    }

    gp_comm->End_frame();
}

#include <print>
#include <iostream>

void ovum::Size_speed_evo_behavior_manager::React_to_event(const eruptor::event::Event & event)
{
    if(auto colision = event.Get_if<eruptor::event::Event::Collision_occurred>())
    {
        if(auto entity = main_scene->Get_if_is_entiety( colision->object_b_id ); main_scene->Get_if_is_food( colision->object_a_id ) && entity)
        {
            entity.value().get().Eat();

            main_scene->Remove_food( colision->object_a_id );
        }
        else if(auto entity = main_scene->Get_if_is_entiety( colision->object_a_id ); entity && main_scene->Get_if_is_food( colision->object_b_id ) )
        {
            entity.value().get().Eat();

            main_scene->Remove_food( colision->object_b_id );
        }

        if(auto entity_a_ = main_scene->Get_if_is_entiety( colision->object_a_id ), entity_b_ = main_scene->Get_if_is_entiety( colision->object_b_id); entity_a_ && entity_b_)
        {
            auto entity_a = entity_a_.value().get();
            auto entity_b = entity_b_.value().get();

            if(entity_a.size >= (1.30 * entity_b.size) && entity_b.ai_data.state == Ai_state::HUNTING)
            {
                std::println(std::clog, "KANIBALIZM!");
                entity_a.Eat();

                main_scene->Remove_entity( entity_b.render_object_id );
            }
            else if(entity_b.size >= (1.30 * entity_a.size) && entity_a.ai_data.state == Ai_state::HUNTING)
            {
                std::println(std::clog, "KANIBALIZM!");
                entity_b.Eat();

                main_scene->Remove_entity( entity_a.render_object_id );
            }
        }
    }
}
