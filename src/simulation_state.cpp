#include <Ovum/simulation_state.hpp>
#include <Ovum/app.hpp>
#include <iostream>

using namespace ovum;

namespace
{
    [[maybe_unused]] std::string_view Ai_state_to_string(Ai_state ai_state)
    {
        switch( ai_state )
        {
            case ovum::Ai_state::HUNTING:
                return "Eating";
            case ovum::Ai_state::RETURN:
                return "Return";
            case ovum::Ai_state::RESTING:
                return "Resting";
            case ovum::Ai_state::DEAD:
                return "Dead";
        }

        return "None";
    }
}

void ovum::Simulation_state::Init(App & app)
{
    Assign_app( app );

    this->main_scene = &app.main_scene;

    gp_comm.Enable_2d_bars("Speed");
    gp_comm.Set_x_axis_title("Speed");
    gp_comm.Set_y_axis_title("Entities count");
    gp_comm.Set_x_axis_range(0.0, 30.0);
    gp_comm.Set_y_axis_range(0, 15);
}

void ovum::Simulation_state::Enter_state()
{
    for(auto & entity : main_scene->entieties)
    {
        entity.ai_data.curr_y_rot = glm::eulerAngles( main_scene->render_objects[ entity.render_object_id ].Get_rotaion() ).y;
        entity.Reset();
    }

    last_time = app->app_clock.now();
}

void ovum::Simulation_state::Update()
{
    std::chrono::duration<float> delta_time = app->app_clock.now() - last_time;

    Update_ai(delta_time.count());
    gp_comm.Begin_frame();

    std::unordered_map<float, uint32_t> entieties_speed{};

    for(auto & entity : main_scene->entieties)
    {
        float bucket = std::round(entity.speed * 10.0f) / 10.0f;
        entieties_speed[bucket]++;
    }

    for(auto [speed, amount] : entieties_speed)
    {
        gp_comm.Stage_data({speed, amount});
    }

    gp_comm.End_frame();

    last_time = app->app_clock.now();
}

void ovum::Simulation_state::Render()
{
    if(app->is_ui_rendered)
    {
        app->renderer->Stage_text_render_data( std::format("Simulation speed: {}", simulation_speed), 10, 80, app->main_font, {55, 20, 130, 255});
    }
}

void ovum::Simulation_state::New_day()
{
    finished_entities = 0;

    while(!main_scene->food.empty())
    {
        main_scene->Remove_food( main_scene->food.back().render_object_id );
    }

    Spawn_food(40);

    auto & entieties = main_scene->entieties;
    auto & render_objects = main_scene->render_objects;
    for(auto i{0UZ}; i < entieties.size(); i++)
    {
        if(entieties[i].ai_data.state == Ai_state::DEAD)
        {
            main_scene->Remove_entity( entieties[i].render_object_id );
            i--;
        }
        else if(entieties[i].ai_data.state == Ai_state::RESTING)
        {
            entieties[i].energy = 30;
            entieties[i].ai_data.state = Ai_state::HUNTING;
            entieties[i].ai_data.time_elapsed = 0;

            if(entieties[i].food_eaten >= 2)
            {
                auto new_id = main_scene->Add_entity();
                entieties[new_id].speed = entieties[i].speed;
                entieties[new_id].speed += evolution_distributor(generator);
                entieties[new_id].ai_data = entieties[i].ai_data;
                entieties[new_id].ai_data.state = Ai_state::HUNTING;

                render_objects[ entieties[new_id].render_object_id ] = render_objects[ entieties[i].render_object_id ];
            }
            entieties[i].food_eaten = 0;
        }
    }
}

void ovum::Simulation_state::Update_ai(float delta_time)
{
    for(auto & entity : main_scene->entieties)
    {
        auto & render_object = main_scene->render_objects[ entity.render_object_id ];

        switch(entity.ai_data.state)
        {
            case Ai_state::HUNTING:
                Update_hunting(render_object, entity, delta_time * simulation_speed);
                break;
            case Ai_state::RETURN:
                Update_return(render_object, entity, delta_time * simulation_speed);
                break;
            default:
                break;
        }
    }
}

void ovum::Simulation_state::Update_hunting(eruptor::scene::Render_object & render_object, Entiety_data & entity_data, float delta_time)
{
    glm::vec3 pos = render_object.Get_position();

    bool near_wall{};
    glm::vec3 desired_dir = render_object.Get_rotaion() * glm::vec3{1.0f, 0.0f, 0.0f};

    if(pos.x > app->world_max.x - wall_margin)
    {
        desired_dir.x = -std::abs(desired_dir.x);
        near_wall = true;
    }
    else if(pos.x < app->world_min.x + wall_margin)
    {
        desired_dir.x = std::abs(desired_dir.x);
        near_wall = true;
    }

    if(pos.z > app->world_max.z - wall_margin)
    {
        desired_dir.z = -std::abs(desired_dir.z);
        near_wall = true;
    }
    else if(pos.z < app->world_min.z + wall_margin)
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

        if(entity_data.ai_data.time_elapsed >= 0.5)
        {
            if(decision_distributor(generator) == 1)
            {
                entity_data.ai_data.is_desire_rot = false;
                entity_data.ai_data.desire_y_rot += rotation_distributor(generator);
                entity_data.ai_data.desire_y_rot = Normilize_angle(entity_data.ai_data.desire_y_rot);;
            }

            entity_data.ai_data.time_elapsed = 0;
        }
    }
    else
    {
        float diff = Normilize_angle( entity_data.ai_data.desire_y_rot - entity_data.ai_data.curr_y_rot );
        if(std::abs(diff) > 0.05f)
        {
            float step = std::copysign(entity_data.speed * delta_time, diff);
            if(std::abs(step) > std::abs(diff))
            {
                step = diff;
            }

            render_object.Rotate({0.0f, step, 0.0f});
            entity_data.ai_data.curr_y_rot = Normilize_angle(entity_data.ai_data.curr_y_rot + step);
        }
        else
        {
            entity_data.ai_data.is_desire_rot = true;
        }
    }

    glm::vec3 forward  = render_object.Get_rotaion() * glm::vec3{1.0f, 0.0f, 0.0f} ;
    render_object.Move( forward * entity_data.speed * delta_time );
    entity_data.energy -= entity_data.speed * delta_time;

    if(entity_data.energy < 5 && entity_data.food_eaten > 0)
    {
       entity_data.ai_data.state = Ai_state::RETURN;
    }
    else if(entity_data.energy <= 0)
    {
        render_object.color = eruptor::resource::Color{255, 255, 255, 255};
        entity_data.ai_data.state = Ai_state::DEAD;

        finished_entities++;
        if(finished_entities >= main_scene->entieties.size())
        {
            New_day();
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

void ovum::Simulation_state::Update_return(eruptor::scene::Render_object& render_object, Entiety_data& entity_data, float delta_time)
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

    float diff = Normilize_angle(entity_data.ai_data.desire_y_rot - entity_data.ai_data.curr_y_rot);

    if(std::abs(diff) > 0.05f)
    {
        float step = std::copysign(entity_data.speed * delta_time, diff);

        if(std::abs(step) > std::abs(diff))
        {
            step = diff;
        }

        render_object.Rotate({0.0f, step, 0.0f});
        entity_data.ai_data.curr_y_rot = Normilize_angle(entity_data.ai_data.curr_y_rot + step);
    }

    glm::vec3 forward = render_object.Get_rotaion() * glm::vec3{1.0f, 0.0f, 0.0f};
    render_object.Move( forward * entity_data.speed * delta_time );
    entity_data.energy -= entity_data.speed * delta_time;

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
            New_day();
        }
    }
    else if(entity_data.energy <= 0)
    {
        render_object.color = eruptor::resource::Color{255, 255, 255, 255};
        entity_data.ai_data.state = Ai_state::DEAD;

        finished_entities++;
        if(finished_entities >= main_scene->entieties.size())
        {
            New_day();
        }
    }
}

void ovum::Simulation_state::Spawn_food(uint32_t food_amount)
{
    float margin{0.6f};
    x_pos_distribution.param( std::uniform_real_distribution<float>::param_type{app->world_min.x + margin, app->world_max.x - margin} );
    z_pos_distribution.param( std::uniform_real_distribution<float>::param_type{app->world_min.z + margin, app->world_max.z - margin} );

    for(auto i{0UZ}; i < food_amount; i++)
    {
        auto id = main_scene->Add_food();
        auto render_id = main_scene->food[ id ].render_object_id;
        main_scene->render_objects[ render_id ].Set_position( {x_pos_distribution(random_device), 0.1f, z_pos_distribution(random_device)} );
    }
}

float ovum::Simulation_state::Normilize_angle(float angle)
{
    return std::atan2(std::sin(angle), std::cos(angle));
}

void ovum::Simulation_state::React_to_event(const eruptor::event::Event & event)
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
    }
    else if(auto mouse_scroll = event.Get_if<eruptor::event::Event::Mouse_scroll>())
    {
        simulation_speed += mouse_scroll->y_offset;
        simulation_speed = std::ceil( simulation_speed );

        if(simulation_speed < 1)
        {
            simulation_speed = 1;
        }

        if(simulation_speed > 100)
        {
            simulation_speed = 100;
        }
    }
    else if(auto key_pressed = event.Get_if<eruptor::event::Event::Key_pressed>())
    {
        switch(key_pressed->key_type)
        {
            case eruptor::event::Key::M:
                app->current_state = &app->editor_state;
                app->current_state->Enter_state();
                break;
            case eruptor::event::Key::SPACE:
                New_day();
                break;
            default:
                break;
        }
    }
}
