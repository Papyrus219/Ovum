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
    gp_comm.Set_x_axis_range(0.0, 50.0);
    gp_comm.Set_y_axis_range(0, 10);
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
    constexpr float fixed_delta_time = 1.0f / 120.0f;
    constexpr float max_frame_durration = 0.025;

    std::chrono::duration<float> delta_time_raw = (app->app_clock.now() - last_time);
    float frame_durration = std::min(delta_time_raw.count(), max_frame_durration);

    time_acumulator += frame_durration;

    while(time_acumulator >= fixed_delta_time)
    {
        Update_ai( fixed_delta_time );

        app->physic_manager->Chceck_colisions( *main_scene, fixed_delta_time );

        time_acumulator -= fixed_delta_time;
    }

    gp_comm.Begin_frame();

    entieties_speed.clear();

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
            simulation_speed = 30;
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
