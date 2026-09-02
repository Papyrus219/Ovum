#include <Ovum/app.hpp>
#include <Eruptor/resource/colors.hpp>
#include <print>
#include <iostream>
#include <format>

using namespace ovum;

ovum::App::App() : event_manager{ eruptor::event::event_manager }
{
    event_manager.Add_listener(*this);
}

void ovum::App::Init()
{
    engine.Init();

    renderer = &engine.Get_renderer();
    resources = &engine.Get_resource_manager();
    physic_manager = &engine.Get_physic_manager();

    resources->texture_dirr_path = "../../textures/";

    window = &renderer->Get_window();
    camera = &renderer->Get_camera();

    main_font = resources->Add_font_atlas("../../fonts/arial.ttf", 30);
    small_font = resources->Add_font_atlas("../../fonts/arial.ttf", 5);

    for(auto & entity : main_scene.entieties)
    {
        entity.ai_data.curr_y_rot = glm::eulerAngles( main_scene.render_objects[ entity.render_object_id ].Get_rotaion() ).y;
        entity.ai_data.curr_y_rot = glm::eulerAngles( main_scene.render_objects[ entity.render_object_id ].Get_rotaion() ).y;
    }

    editor_state.Init( *this );
    simulation_state.Init( *this );

    resources->Load_resources();

    auto floor_it = main_scene.objects_aliases.find("floor");

    for(auto & alias : main_scene.objects_aliases)
    {
        std::println(std::clog, "{} : {}", alias.first, alias.second);
    }

    if(floor_it != main_scene.objects_aliases.end())
    {
        auto floor_aabb = main_scene.render_objects[ floor_it->second ].Get_aabb();

        world_min = floor_aabb.min;
        world_max = floor_aabb.max;

        std::println(std::clog, "Find world boundries: min: {} {} {} max: {} {} {}", world_min.x, world_min.y, world_min.z, world_max.x, world_max.y, world_max.z);
    }

    current_state = &editor_state;
    last_time = app_clock.now();
}

void ovum::App::Start_loop()
{
    while(is_running)
    {
        Update();

        Render();
    }
}

void ovum::App::Update()
{
    window->Update();
    physic_manager->Chceck_colisions( main_scene );

    std::chrono::duration<float> delta_time = app_clock.now() - last_time;

    std::println(std::clog, "FPS: {}", 1.0f / delta_time.count());

    if(window->Is_key_pressed(eruptor::event::Key::W))
    {
        camera->Process_keyboard(eruptor::renderer::Camera_movement_direction::FORWARD, delta_time.count());
    }
    if(window->Is_key_pressed(eruptor::event::Key::S))
    {
        camera->Process_keyboard(eruptor::renderer::Camera_movement_direction::BACKWARD, delta_time.count());
    }
    if(window->Is_key_pressed(eruptor::event::Key::A))
    {
        camera->Process_keyboard(eruptor::renderer::Camera_movement_direction::LEFT, delta_time.count());
    }
    if(window->Is_key_pressed(eruptor::event::Key::D))
    {
        camera->Process_keyboard(eruptor::renderer::Camera_movement_direction::RIGHT, delta_time.count());
    }

    current_state->Update();
    last_time = app_clock.now();
}

void ovum::App::Render()
{
    renderer->Stage_scene_render_data( main_scene );

    if(is_ui_rendered)
    {
        renderer->Stage_text_render_data( std::format("Mode: {}", current_state->Get_state_name()), 10, 40, main_font, {155, 10, 10, 255});
    }

    current_state->Render();
    renderer->Flush_render_buffor();
}

void ovum::App::On_event(const eruptor::event::Event & event)
{
    if(event.Is<eruptor::event::Event::Close_window>())
    {
        is_running = false;
    }
    else if(auto mouse_move = event.Get_if<eruptor::event::Event::Mouse_moved>())
    {
        camera->Process_mouse_movement(mouse_move->x_offset, mouse_move->y_offset);
    }
    else if(auto key_pressed = event.Get_if<eruptor::event::Event::Key_pressed>())
    {
        if(key_pressed->key_type == eruptor::event::Key::F2)
        {
            is_ui_rendered = !is_ui_rendered;
        }
    }

    current_state->React_to_event( event );
}


