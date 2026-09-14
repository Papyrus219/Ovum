#ifndef OVUM_EDITOR_STATE_HPP
#define OVUM_EDITOR_STATE_HPP

#include <Ovum/app_state.hpp>
#include <Eruptor/resource/scene_parser.hpp>
#include <Eruptor/scene/scene_saver.hpp>
#include <Eruptor/event/event_manager.hpp>
#include <Ovum/simulation_parser.hpp>
#include <Ovum/simulation_saver.hpp>

namespace ovum
{

class Editor_state : public ovum::App_state
{
public:
    virtual void Init(App & app) override;
    virtual void Enter_state() override;

    virtual void Update() override;
    virtual void Render() override;
    virtual void React_to_event(const eruptor::event::Event& event) override;

    virtual std::string_view Get_state_name() override {return "Editor";}

    ovum::Simulation_scene * main_scene{};

    enum class Object_type
    {
        ENTITY,
        FOOD,
        LIGHT_SOURCE
    } object_type;

    void Reload_scene();
    void Save_scene();

    void Handle_file_key_input(eruptor::event::Key key);

    std::string_view Get_string_from_object_type_enum(Object_type type);

    std::optional<uint32_t> current_entity_selected{};
    std::optional<uint32_t> current_food_selected{};
    std::optional<uint32_t> current_light_source_selected{};

    float enemy_scroll_offset{};
    float food_scroll_offset{};
    float light_source_scroll_offset{};

    eruptor::scene::Scene_saver scene_saver{};
    ovum::Simulation_saver simulation_saver{};

    std::chrono::high_resolution_clock::time_point last_time{};

    static const std::filesystem::path scene_path_1;
    static const std::filesystem::path scene_path_2;
    static const std::filesystem::path scene_path_3;
    static const std::filesystem::path scene_path_4;
    static const std::filesystem::path scene_path_5;
    static const std::filesystem::path scene_path_6;
    static const std::filesystem::path scene_path_7;
    static const std::filesystem::path scene_path_8;
    static const std::filesystem::path scene_path_9;

    static const std::filesystem::path simulation_path_1;
    static const std::filesystem::path simulation_path_2;
    static const std::filesystem::path simulation_path_3;
    static const std::filesystem::path simulation_path_4;
    static const std::filesystem::path simulation_path_5;
    static const std::filesystem::path simulation_path_6;
    static const std::filesystem::path simulation_path_7;
    static const std::filesystem::path simulation_path_8;
    static const std::filesystem::path simulation_path_9;
};

}

#endif // OVUM_EDITOR_STATE_HPP
