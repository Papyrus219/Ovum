#ifndef OVUM_SIMULATION_PARSER_HPP
#define OVUM_SIMULATION_PARSER_HPP

#include <expected>
#include <string>
#include <string_view>
#include <cstdint>
#include <Ovum/simulation_scene.hpp>

namespace eruptor::physic
{
    class Physic_manager;
}

namespace ovum
{

class Simulation_parser
{
public:
    void Init(eruptor::physic::Physic_manager & physic_manager);
    std::expected<Simulation_scene, std::string_view> Load_simulation_data_into_scene(const std::filesystem::path & path, eruptor::scene::Scene & base_scene);

private:
    void Load_file_to_buffor(const std::filesystem::path & path);
    void Parse_line(std::string_view line, Simulation_scene & scene);

    std::string buffor{};
    uint32_t line_count{};

    enum class Line_mode
    {
        NONE,
        VERSION_CHECK,
        OBJECT_DECLARATION,
        ENTITY_SPEED_DECLARATION,
    } line_mode{};

    enum class Version
    {
        V1_0
    } file_version{};

    bool error_happen{};
    std::string_view error_message{};

    Entiety_data current_parsed_entity_data{};
    Food_data current_parsed_food_data{};
    Light_source_data current_parsed_light_source_data{};

    static const std::string error_file_load;
    static const std::string error_text_parsing;

    eruptor::physic::Physic_manager * physic_manager{};
};

}

#endif // OVUM_SIMULATION_PARSER_HPP
