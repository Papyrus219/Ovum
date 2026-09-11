#include <Ovum/editor_state.hpp>
#include <Ovum/app.hpp>
#include <Eruptor/scene/scene.hpp>
#include <print>
#include <iostream>

using namespace ovum;

const std::filesystem::path ovum::Editor_state::scene_path_1{"../../scenes/test_1.papsc"};
const std::filesystem::path ovum::Editor_state::scene_path_2{"../../scenes/test_2.papsc"};
const std::filesystem::path ovum::Editor_state::scene_path_3{"../../scenes/test_3.papsc"};
const std::filesystem::path ovum::Editor_state::scene_path_4{"../../scenes/test_4.papsc"};
const std::filesystem::path ovum::Editor_state::scene_path_5{"../../scenes/test_5.papsc"};
const std::filesystem::path ovum::Editor_state::scene_path_6{"../../scenes/test_6.papsc"};
const std::filesystem::path ovum::Editor_state::scene_path_7{"../../scenes/test_7.papsc"};
const std::filesystem::path ovum::Editor_state::scene_path_8{"../../scenes/test_8.papsc"};
const std::filesystem::path ovum::Editor_state::scene_path_9{"../../scenes/test_9.papsc"};

const std::filesystem::path ovum::Editor_state::simulation_path_1{"../../simulations/test_1.papsim"};
const std::filesystem::path ovum::Editor_state::simulation_path_2{"../../simulations/test_2.papsim"};
const std::filesystem::path ovum::Editor_state::simulation_path_3{"../../simulations/test_3.papsim"};
const std::filesystem::path ovum::Editor_state::simulation_path_4{"../../simulations/test_4.papsim"};
const std::filesystem::path ovum::Editor_state::simulation_path_5{"../../simulations/test_5.papsim"};
const std::filesystem::path ovum::Editor_state::simulation_path_6{"../../simulations/test_6.papsim"};
const std::filesystem::path ovum::Editor_state::simulation_path_7{"../../simulations/test_7.papsim"};
const std::filesystem::path ovum::Editor_state::simulation_path_8{"../../simulations/test_8.papsim"};
const std::filesystem::path ovum::Editor_state::simulation_path_9{"../../simulations/test_9.papsim"};

void ovum::Editor_state::Init(App & app)
{
    Assign_app(app);

    scene_saver.Assign_resource_manager(*app.resources);

    this->main_scene = &app.main_scene;
    Reload_scene();
}

void ovum::Editor_state::Enter_state()
{
    last_time = app->app_clock.now();
}

void ovum::Editor_state::Update()
{
    std::chrono::duration<float> delta_time = app->app_clock.now() - last_time;
    float move_speed{0.2f};
    float rotation_speed{ 0.05f };

    if(object_type == Object_type::ENTITY && current_entity_selected.has_value())
    {
        if(app->window->Is_key_pressed(eruptor::event::Key::DOWN))
        {
            main_scene->render_objects[ main_scene->entieties[ *current_entity_selected ].render_object_id ].Move( glm::vec3{0.0f, 0.0f, 1.0f} * move_speed  );
        }
        if(app->window->Is_key_pressed(eruptor::event::Key::UP))
        {
            main_scene->render_objects[ main_scene->entieties[ *current_entity_selected ].render_object_id ].Move( glm::vec3{0.0f, 0.0f, -1.0f} * move_speed  );
        }
        if(app->window->Is_key_pressed(eruptor::event::Key::RIGHT))
        {
            main_scene->render_objects[ main_scene->entieties[ *current_entity_selected ].render_object_id ].Move( glm::vec3{1.0f, 0.0f, 0.0f} * move_speed  );
        }
        if(app->window->Is_key_pressed(eruptor::event::Key::LEFT))
        {
            main_scene->render_objects[ main_scene->entieties[ *current_entity_selected ].render_object_id ].Move( glm::vec3{-1.0f, 0.0f, 0.0f} * move_speed  );
        }

        if(app->window->Is_key_pressed(eruptor::event::Key::Q))
        {
            main_scene->render_objects[ main_scene->entieties[ *current_entity_selected ].render_object_id ].Rotate( glm::vec3{0.0f, 1.0f, 0.0f} * rotation_speed );
        }
        if(app->window->Is_key_pressed(eruptor::event::Key::E))
        {
            main_scene->render_objects[ main_scene->entieties[ *current_entity_selected ].render_object_id ].Rotate( glm::vec3{0.0f, -1.0f, 0.0f} * rotation_speed );
        }
    }
    else if(object_type == Object_type::FOOD && current_food_selected.has_value())
    {
        if(app->window->Is_key_pressed(eruptor::event::Key::DOWN))
        {
            main_scene->render_objects[ main_scene->food[ *current_food_selected ].render_object_id ].Move( glm::vec3{0.0f, 0.0f, 1.0f} * move_speed  );
        }
        if(app->window->Is_key_pressed(eruptor::event::Key::UP))
        {
            main_scene->render_objects[ main_scene->food[ *current_food_selected ].render_object_id ].Move( glm::vec3{0.0f, 0.0f, -1.0f} * move_speed  );
        }
        if(app->window->Is_key_pressed(eruptor::event::Key::RIGHT))
        {
            main_scene->render_objects[ main_scene->food[ *current_food_selected ].render_object_id ].Move( glm::vec3{1.0f, 0.0f, 0.0f} * move_speed  );
        }
        if(app->window->Is_key_pressed(eruptor::event::Key::LEFT))
        {
            main_scene->render_objects[ main_scene->food[ *current_food_selected ].render_object_id ].Move( glm::vec3{-1.0f, 0.0f, 0.0f} * move_speed  );
        }

        if(app->window->Is_key_pressed(eruptor::event::Key::Q))
        {
            main_scene->render_objects[ main_scene->food[ *current_food_selected ].render_object_id ].Rotate( glm::vec3{0.0f, 1.0f, 0.0f} * rotation_speed );
        }
        if(app->window->Is_key_pressed(eruptor::event::Key::E))
        {
            main_scene->render_objects[ main_scene->food[ *current_food_selected ].render_object_id ].Rotate( glm::vec3{0.0f, -1.0f, 0.0f} * rotation_speed );
        }
    }
    else if(object_type == Object_type::LIGHT_SOURCE && current_light_source_selected.has_value())
    {
        if(app->window->Is_key_pressed(eruptor::event::Key::DOWN))
        {
            main_scene->render_objects[ main_scene->light_sources[ *current_light_source_selected ].render_object_id ].Move( glm::vec3{0.0f, 0.0f, 1.0f} * move_speed  );
        }
        if(app->window->Is_key_pressed(eruptor::event::Key::UP))
        {
            main_scene->render_objects[ main_scene->light_sources[ *current_light_source_selected ].render_object_id ].Move( glm::vec3{0.0f, 0.0f, -1.0f} * move_speed  );
        }
        if(app->window->Is_key_pressed(eruptor::event::Key::RIGHT))
        {
            main_scene->render_objects[ main_scene->light_sources[ *current_light_source_selected ].render_object_id ].Move( glm::vec3{1.0f, 0.0f, 0.0f} * move_speed  );
        }
        if(app->window->Is_key_pressed(eruptor::event::Key::LEFT))
        {
            main_scene->render_objects[ main_scene->light_sources[ *current_light_source_selected ].render_object_id ].Move( glm::vec3{-1.0f, 0.0f, 0.0f} * move_speed  );
        }
        if(app->window->Is_key_pressed(eruptor::event::Key::E))
        {
            main_scene->render_objects[ main_scene->light_sources[ *current_light_source_selected ].render_object_id ].Move( glm::vec3{0.0f, 1.0f, 0.0f} * move_speed );
        }
        if(app->window->Is_key_pressed(eruptor::event::Key::Q))
        {
            main_scene->render_objects[ main_scene->light_sources[ *current_light_source_selected ].render_object_id ].Move( glm::vec3{0.0f, -1.0f, 0.0f} * move_speed );
        }
    }

    last_time = app->app_clock.now();
}

void ovum::Editor_state::Render()
{
    if(app->is_ui_rendered)
    {
        app->renderer->Stage_text_render_data( std::format("Selected object type: {}", Get_string_from_object_type_enum( object_type )), 10, 80, app->main_font, {20, 40, 155, 255});

        app->renderer->Stage_text_render_data("M - Change mode", 890, 820, app->small_font, {255, 255, 255, 255});
        app->renderer->Stage_text_render_data("T - Change object type", 882, 850, app->small_font, {255, 255, 255, 255});
        app->renderer->Stage_text_render_data("N - Add new object", 882, 880, app->small_font, {255, 255, 255, 255});
    }
}

void ovum::Editor_state::Reload_scene()
{
    auto parsed_scene = app->scene_parser.Load_scene(app->current_scene_path);
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

void ovum::Editor_state::Save_scene()
{
    auto result = scene_saver.Save_scene_data(*main_scene, app->current_scene_path);
    if(!result.has_value())
    {
        std::println(std::cerr, "Error: {}", result.error());
        std::exit(EXIT_FAILURE);
    }

    result = simulation_saver.Save_simulation_data(*main_scene, app->current_simulation_info_path);
    if(!result.has_value())
    {
        std::println(std::cerr, "Error: {}", result.error());
        std::exit(EXIT_FAILURE);
    }
}

std::string_view ovum::Editor_state::Get_string_from_object_type_enum(Object_type type)
{
    switch(type)
    {
        case Object_type::ENTITY:
            return "Entity";
        case Object_type::FOOD:
            return "Food";
        case Object_type::LIGHT_SOURCE:
            return "Light source";
    }

    return "None";
}

void ovum::Editor_state::React_to_event(const eruptor::event::Event & event)
{
    if(auto key_pressed = event.Get_if<eruptor::event::Event::Key_pressed>())
    {
        switch(key_pressed->key_type)
        {
            case eruptor::event::Key::M:
                app->current_state = &app->simulation_state;
                app->current_state->Enter_state();
                break;
            case eruptor::event::Key::N:
                if(object_type == Object_type::ENTITY)
                {
                    auto id = main_scene->Add_entity();

                    if(current_entity_selected.has_value())
                    {
                        main_scene->render_objects[ main_scene->entieties[ *current_entity_selected ].render_object_id ].is_selected = false;
                    }

                    current_entity_selected = id;
                    main_scene->render_objects[ main_scene->entieties[ *current_entity_selected ].render_object_id ].is_selected = true;
                }
                else if(object_type == Object_type::FOOD)
                {
                    auto id = main_scene->Add_food();

                    if(current_food_selected.has_value())
                    {
                        main_scene->render_objects[ main_scene->food[ *current_food_selected ].render_object_id ].is_selected = false;
                    }

                    current_food_selected = id;
                    main_scene->render_objects[ main_scene->food[ *current_food_selected ].render_object_id ].is_selected = true;
                }
                else if(object_type == Object_type::LIGHT_SOURCE)
                {
                    auto id = main_scene->Add_light_source();

                    if(current_light_source_selected.has_value())
                    {
                        main_scene->render_objects[ main_scene->entieties[ *current_light_source_selected ].render_object_id ].is_selected = false;
                    }

                    current_light_source_selected = id;
                    main_scene->render_objects[ main_scene->light_sources[ *current_light_source_selected ].render_object_id ].is_selected = true;
                }
                break;
            case eruptor::event::Key::DEL:
                if(object_type == Object_type::ENTITY)
                {
                    if(current_entity_selected.has_value())
                    {
                        main_scene->Remove_entity( *current_entity_selected );

                        if(main_scene->entieties.empty())
                        {
                            current_entity_selected.reset();
                        }
                        else
                        {
                            current_entity_selected = main_scene->entieties.size() - 1;
                            main_scene->render_objects[ main_scene->entieties[ *current_entity_selected ].render_object_id ].is_selected = true;
                            if(enemy_scroll_offset >= main_scene->entieties.size()) enemy_scroll_offset = static_cast<float>( main_scene->entieties.size() ) - 0.001;
                        }
                    }
                }
                else if(object_type == Object_type::FOOD)
                {
                    if(current_food_selected.has_value())
                    {
                        main_scene->Remove_food( *current_food_selected );

                        if(main_scene->food.empty())
                        {
                            current_food_selected.reset();
                        }
                        else
                        {
                            current_food_selected = main_scene->food.size() - 1;
                            main_scene->render_objects[ main_scene->food[ *current_food_selected ].render_object_id ].is_selected = true;
                            if(food_scroll_offset >= main_scene->food.size()) food_scroll_offset = static_cast<float>( main_scene->food.size() ) - 0.001;
                        }
                    }
                }
                else if(object_type == Object_type::LIGHT_SOURCE)
                {
                    if(current_light_source_selected.has_value())
                    {
                        main_scene->Remove_light_source( *current_light_source_selected );

                        if(main_scene->light_sources.empty())
                        {
                            current_food_selected.reset();
                        }
                        else
                        {
                            current_light_source_selected = main_scene->light_sources.size() - 1;
                            main_scene->render_objects[ main_scene->light_sources[ *current_light_source_selected ].render_object_id ].is_selected = true;
                            if(light_source_scroll_offset >= main_scene->light_sources.size()) light_source_scroll_offset = static_cast<float>( main_scene->light_sources.size() ) - 0.001;
                        }
                    }
                }
                break;
            case eruptor::event::Key::T:
                if(object_type == Object_type::ENTITY)
                {
                    object_type = Object_type::FOOD;
                }
                else if(object_type == Object_type::FOOD)
                {
                    object_type = Object_type::LIGHT_SOURCE;
                }
                else if(object_type == Object_type::LIGHT_SOURCE)
                {
                    object_type = Object_type::ENTITY;
                }
                break;
            default:
                break;
        }

        Handle_file_key_input( key_pressed->key_type );
    }
    else if(auto scroll = event.Get_if<eruptor::event::Event::Mouse_scroll>())
    {
        if(object_type == Object_type::ENTITY && current_entity_selected.has_value())
        {
            enemy_scroll_offset += scroll->y_offset;

            if(enemy_scroll_offset < 0) enemy_scroll_offset = 0;
            if(enemy_scroll_offset >= main_scene->entieties.size()) enemy_scroll_offset = static_cast<float>( main_scene->entieties.size() ) - 0.001;

            main_scene->render_objects[ main_scene->entieties[ *current_entity_selected ].render_object_id ].is_selected = false;
            current_entity_selected = static_cast<uint32_t>(enemy_scroll_offset);
            main_scene->render_objects[ main_scene->entieties[ *current_entity_selected ].render_object_id ].is_selected = true;

            std::println(std::clog, "Entity_scroll: {}", enemy_scroll_offset);
        }
        else if(object_type == Object_type::FOOD && current_food_selected.has_value())
        {
            food_scroll_offset += scroll->y_offset;

            if(food_scroll_offset < 0) food_scroll_offset = 0;
            if(food_scroll_offset >= main_scene->food.size()) light_source_scroll_offset = static_cast<float>( main_scene->food.size() ) - 0.001;

            main_scene->render_objects[ main_scene->food[ *current_food_selected ].render_object_id ].is_selected = false;
            current_food_selected = static_cast<uint32_t>(food_scroll_offset);
            main_scene->render_objects[ main_scene->food[ *current_food_selected ].render_object_id ].is_selected = true;
        }
        else if(object_type == Object_type::LIGHT_SOURCE && current_light_source_selected.has_value())
        {
            light_source_scroll_offset += scroll->y_offset;

            if(light_source_scroll_offset < 0) light_source_scroll_offset = 0;
            if(light_source_scroll_offset >= main_scene->food.size()) light_source_scroll_offset = static_cast<float>( main_scene->light_sources.size() ) - 0.001;

            main_scene->render_objects[ main_scene->light_sources[ *current_light_source_selected ].render_object_id ].is_selected = false;
            current_light_source_selected = static_cast<uint32_t>(light_source_scroll_offset);
            main_scene->render_objects[ main_scene->light_sources[ *current_light_source_selected ].render_object_id ].is_selected = true;
        }
    }
}

void ovum::Editor_state::Handle_file_key_input(eruptor::event::Key key)
{
    switch(key)
    {
        case eruptor::event::Key::R:
            Reload_scene();
            break;
        case eruptor::event::Key::TAB:
            Save_scene();
            break;
        case eruptor::event::Key::KEY_1:
            if(app->window->Is_key_pressed( eruptor::event::Key::LEFT_SHIFT ))
            {
                app->current_simulation_info_path = simulation_path_1;
            }
            else
            {
                app->current_scene_path = scene_path_1;
            }
            std::println(std::clog, "Current scene_path: {}\nCurrent simulation_path: {}", app->current_scene_path.c_str(), app->current_simulation_info_path.c_str());
            break;
        case eruptor::event::Key::KEY_2:
            if(app->window->Is_key_pressed( eruptor::event::Key::LEFT_SHIFT ))
            {
                app->current_simulation_info_path = simulation_path_2;
            }
            else
            {
                app->current_scene_path = scene_path_2;
            }
            std::println(std::clog, "Current scene_path: {}\nCurrent simulation_path: {}", app->current_scene_path.c_str(), app->current_simulation_info_path.c_str());
            break;
        case eruptor::event::Key::KEY_3:
            if(app->window->Is_key_pressed( eruptor::event::Key::LEFT_SHIFT ))
            {

                app->current_simulation_info_path = simulation_path_3;
            }
            else
            {
                app->current_scene_path = scene_path_3;
            }
            std::println(std::clog, "Current scene_path: {}\nCurrent simulation_path: {}", app->current_scene_path.c_str(), app->current_simulation_info_path.c_str());
            break;
        case eruptor::event::Key::KEY_4:
            if(app->window->Is_key_pressed( eruptor::event::Key::LEFT_SHIFT ))
            {
                app->current_simulation_info_path = simulation_path_4;
            }
            else
            {
                app->current_scene_path = scene_path_4;
            }
            std::println(std::clog, "Current scene_path: {}\nCurrent simulation_path: {}", app->current_scene_path.c_str(), app->current_simulation_info_path.c_str());
            break;
        case eruptor::event::Key::KEY_5:
            if(app->window->Is_key_pressed( eruptor::event::Key::LEFT_SHIFT ))
            {
                app->current_simulation_info_path = simulation_path_5;
            }
            else
            {
                app->current_scene_path = scene_path_5;
            }
            std::println(std::clog, "Current scene_path: {}\nCurrent simulation_path: {}", app->current_scene_path.c_str(), app->current_simulation_info_path.c_str());
            break;
        case eruptor::event::Key::KEY_6:
            if(app->window->Is_key_pressed( eruptor::event::Key::LEFT_SHIFT ))
            {
                app->current_simulation_info_path = simulation_path_6;
            }
            else
            {
                app->current_scene_path = scene_path_6;
            }
            std::println(std::clog, "Current scene_path: {}\nCurrent simulation_path: {}", app->current_scene_path.c_str(), app->current_simulation_info_path.c_str());
            break;
        case eruptor::event::Key::KEY_7:
            if(app->window->Is_key_pressed( eruptor::event::Key::LEFT_SHIFT ))
            {
                app->current_simulation_info_path = simulation_path_7;
            }
            else
            {
                app->current_scene_path = scene_path_7;
            }
            std::println(std::clog, "Current scene_path: {}\nCurrent simulation_path: {}", app->current_scene_path.c_str(), app->current_simulation_info_path.c_str());
            break;
        case eruptor::event::Key::KEY_8:
            if(app->window->Is_key_pressed( eruptor::event::Key::LEFT_SHIFT ))
            {
                app->current_simulation_info_path = simulation_path_8;
            }
            else
            {
                app->current_scene_path = scene_path_8;
            }
            std::println(std::clog, "Current scene_path: {}\nCurrent simulation_path: {}", app->current_scene_path.c_str(), app->current_simulation_info_path.c_str());
            break;
        case eruptor::event::Key::KEY_9:
            if(app->window->Is_key_pressed( eruptor::event::Key::LEFT_SHIFT ))
            {
                app->current_simulation_info_path = simulation_path_9;
            }
            else
            {
                app->current_scene_path = scene_path_9;
            }
            std::println(std::clog, "Current scene_path: {}\nCurrent simulation_path: {}", app->current_scene_path.c_str(), app->current_simulation_info_path.c_str());
            break;
        default:
            break;
    }
}


