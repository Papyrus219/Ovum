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

    behavior_manager.Init( *this );

    auto & parser = app.resources->config_parser;
    auto config_data = parser.Parse_config_file("../../configuration/simulation.papcfg");

    if(config_data)
    {
        if(config_data->at("Speed evo") == "ON")
        {
            sim_settings.is_speed_evo_enabled = true;

            float min{}, max{};
            parser.Convert_string_to_number(config_data->at("Speed evo range min"), min, "Speed evo range min");
            parser.Convert_string_to_number(config_data->at("Speed evo range max"), max, "Speed evo range max");

            sim_settings.speed_evo_distributor.param( std::uniform_real_distribution<float>::param_type{ min, max } );
        }
        if(config_data->at("Size evo") == "ON")
        {
            sim_settings.is_size_evo_enabled = true;

            float min{}, max{};
            parser.Convert_string_to_number(config_data->at("Size evo range min"), min, "Size evo range min");
            parser.Convert_string_to_number(config_data->at("Size evo range max"), max, "Size evo range max");

            sim_settings.size_evo_distributor.param( std::uniform_real_distribution<float>::param_type{ min, max } );
        }
        if(config_data->at("Sense evo") == "ON")
        {
            sim_settings.is_sense_evo_enabled = true;

            float min{}, max{};
            parser.Convert_string_to_number(config_data->at("Sense evo range min"), min, "Sense evo range min");
            parser.Convert_string_to_number(config_data->at("Sense evo range max"), max, "Sense evo range max");

            sim_settings.sense_evo_distributor.param( std::uniform_real_distribution<float>::param_type{ min, max } );
        }

        if(config_data->at("Use sense with movement") == "ON")
        {
            sim_settings.is_sense_used = true;
        }
        if(config_data->at("Canibalism") == "ON")
        {
            sim_settings.is_canibalism_enabled = true;
        }

        if(config_data->at("Hunger can be satisfied") == "ON")
        {
            sim_settings.is_hunger_can_be_satisfied = true;
        }

        registry = Make_registry();
        formula = Formula::Compile(config_data->at("Energy formula"), registry);

        parser.Convert_string_to_number(config_data->at("Start energy"), sim_settings.start_energy, "Start energy");
        parser.Convert_string_to_number(config_data->at("Food per day"), sim_settings.food_per_day, "Food per day");
        parser.Convert_string_to_number(config_data->at("Survive need"), sim_settings.food_survive_need, "Survive need");
        parser.Convert_string_to_number(config_data->at("Reproduction need"), sim_settings.food_reproduction_need, "Reproduction need");
    }
}

void ovum::Simulation_state::Enter_state()
{
    for(auto & entity : main_scene->entieties)
    {
        entity.Reset();

        glm::vec3 forward = main_scene->render_objects[ entity.render_object_id ].Get_rotation() * glm::vec3{1.0f, 0.0f, 0.0f};
        float yaw = std::atan2(-forward.z, forward.x);

        entity.ai_data.curr_y_rot = yaw;
        entity.ai_data.desire_y_rot = yaw;
        entity.ai_data.is_desire_rot = true;
    }

    last_time = app->app_clock.now();
    Update_graph();
    behavior_manager.Update_graph();
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
        behavior_manager.Update_ai( fixed_delta_time );

        app->physic_manager->Update_scene(*main_scene, fixed_delta_time);
        time_acumulator -= fixed_delta_time;
    }

    last_time = app->app_clock.now();
}

void ovum::Simulation_state::Update_graph()
{
    population.push_back( app->main_scene.entieties.size() );
    day_passed++;

    if(population.size() > 30)
    {
        population.erase( population.begin() );
    }

    gp_comm_pop.Begin_frame();
    for(auto i{0UZ}; i < 30; i++)
    {
        gp_comm_pop.Stage_data({ (day_passed > 30)? day_passed - (30 - i) : i , (population.size() > i) ? population[i] : 0});
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
    behavior_manager.React_to_event(event);

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
                behavior_manager.New_day();
                break;
            default:
                break;
        }
    }
}
