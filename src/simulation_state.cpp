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

    gp_comm_pop.Enable_2d_bars("POPULATION", false);
    gp_comm_pop.Set_x_axis_title("Day");
    gp_comm_pop.Set_y_axis_title("Population");

    this->main_scene = &app.main_scene;

    sense_speed_size_evo_behavior.Init( *this );
    size_speed_evo_behavior.Init( *this );
    speed_evo_behavior.Init( *this );
    survive_behavior.Init( *this );
    Set_entity_behavior(sense_speed_size_evo_behavior);

    auto & parser = app.resources->config_parser;
    auto config_data = parser.Parse_config_file("../../configuration/simulation.papcfg");

    if(config_data)
    {
        registry = Make_registry();
        formula = Formula::Compile(config_data->at("Energy formula"), registry);

        parser.Convert_string_to_number(config_data->at("Start energy"), start_energy, "Start energy");
        parser.Convert_string_to_number(config_data->at("Food per day"), food_per_day, "Food per day");
        parser.Convert_string_to_number(config_data->at("Survive need"), food_survive_need, "Survive need");
        parser.Convert_string_to_number(config_data->at("Reproduction need"), food_reproduction_need, "Reproduction need");
    }
}

void ovum::Simulation_state::Enter_state()
{
    for(auto & entity : main_scene->entieties)
    {
        entity.ai_data.curr_y_rot = glm::eulerAngles( main_scene->render_objects[ entity.render_object_id ].Get_rotation() ).y;
        entity.Reset();
    }

    last_time = app->app_clock.now();
    Update_graph();
    behavior_manager->Update_graph();
}

void ovum::Simulation_state::Update()
{
    constexpr float fixed_delta_time = 1.0f / 60.0f;
    constexpr float max_frame_durration = 0.025;

    std::chrono::duration<float> delta_time_raw = (app->app_clock.now() - last_time);
    float frame_durration = std::min(delta_time_raw.count(), max_frame_durration);

    time_acumulator += frame_durration * simulation_speed;

    while(time_acumulator >= fixed_delta_time)
    {
        behavior_manager->Update_ai( fixed_delta_time );

        app->physic_manager->Update_scene(*main_scene, fixed_delta_time);
        time_acumulator -= fixed_delta_time;
    }

    last_time = app->app_clock.now();
}

void ovum::Simulation_state::Update_graph()
{
    population.push_back( app->main_scene.entieties.size() );

    gp_comm_pop.Begin_frame();
    for(auto i{0UZ}; i < population.size(); i++)
    {
        gp_comm_pop.Stage_data({i, population[i]});
    }
    gp_comm_pop.End_frame();
}

void ovum::Simulation_state::Render()
{
    if(app->is_ui_rendered)
    {
        app->renderer->Stage_text_render_data( std::format("Simulation speed: {}", simulation_speed), 10, 80, app->main_font, {55, 20, 130, 255});
    }
}

void ovum::Simulation_state::Set_entity_behavior(Entity_behavior_manager & entity_behavior)
{
    Reload_scene();
    this->behavior_manager = &entity_behavior;
    this->behavior_manager->Setup();
}

float ovum::Simulation_state::Normilize_angle(float angle)
{
    return std::atan2(std::sin(angle), std::cos(angle));
}

void ovum::Simulation_state::Reload_scene()
{
    auto parsed_scene = app->resources->scene_parser.Load_scene(app->current_scene_path);
    if(parsed_scene)
    {
        *main_scene = *parsed_scene;
        main_scene->Init( *app->resources );
    }
    else
    {
        std::print(std::cerr, "Error: {}\n", parsed_scene.error());
    }

    auto parsed_simulation_scene = app->simulation_parser.Load_simulation_data_into_scene(app->current_simulation_info_path, *main_scene);
    if(parsed_simulation_scene.has_value())
    {
        *main_scene = parsed_simulation_scene.value();
        main_scene->Init( *app->resources );
    }
    else
    {
        std::println(std::cerr, "Error: {}", parsed_simulation_scene.error());
        std::exit(EXIT_FAILURE);
    }
}

void ovum::Simulation_state::React_to_event(const eruptor::event::Event & event)
{
    behavior_manager->React_to_event(event);

    if(auto mouse_scroll = event.Get_if<eruptor::event::Event::Mouse_scroll>())
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
                behavior_manager->New_day();
                break;
            default:
                break;
        }
    }
}
