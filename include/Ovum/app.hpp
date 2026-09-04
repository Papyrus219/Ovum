#ifndef OVUM_APP_HPP
#define OVUM_APP_HPP

#include <Eruptor/eruptor.hpp>
#include <Ovum/editor_state.hpp>
#include <Ovum/simulation_state.hpp>
#include <Ovum/simulation_scene.hpp>
#include <Ovum/gp_communicator.hpp>

namespace ovum
{

class App : public eruptor::event::Event_listener
{
public:
    App();
    void Init();

    void Start_loop();

    virtual void On_event(const eruptor::event::Event & event) override;

    void Update();
    void Render();

    bool is_running{true};

    ovum::Simulation_scene main_scene{};

    eruptor::resource::Font_handle main_font{};
    eruptor::resource::Font_handle small_font{};

    float time_elapsed{};
    bool is_ui_rendered{true};

    std::chrono::high_resolution_clock app_clock{};
    std::chrono::high_resolution_clock::time_point last_time{};

    glm::vec3 world_min{}, world_max{};

    eruptor::Eruptor engine{};

    eruptor::renderer::Renderer * renderer{};
    eruptor::resource::Resource_manager * resources{};
    eruptor::event::Event_manager & event_manager;
    eruptor::physic::Physic_manager * physic_manager{};

    eruptor::hardware::Window * window{};
    eruptor::renderer::Fly_camera * camera{};

    App_state * current_state{};
    Editor_state editor_state{};
    Simulation_state simulation_state{};

    eruptor::scene::Scene_parser scene_parser{};
    ovum::Simulation_parser simulation_parser{};

    std::filesystem::path current_scene_path{Editor_state::scene_path_1};
    std::filesystem::path current_simulation_info_path{Editor_state::simulation_path_1};

    friend class App_state;
};

}

#endif // OVUM_APP_HPP
