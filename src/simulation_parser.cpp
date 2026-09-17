#include <Ovum/simulation_parser.hpp>
#include <Eruptor/physic_manager.hpp>
#include <iostream>
#include <fstream>
#include <print>

#ifndef NDEBUG
constexpr bool Debug_mode = true;
#else
constexpr bool Debug_mode = false;
#endif

const std::string ovum::Simulation_parser::error_file_load{"Simulation parser: Failed to load file!"};
const std::string ovum::Simulation_parser::error_text_parsing{"Simulation parser: Failed to parse text correctly!"};

void ovum::Simulation_parser::Init(eruptor::physic::Physic_manager & physic_manager)
{
    this->physic_manager = &physic_manager;
}

std::expected<ovum::Simulation_scene, std::string_view> ovum::Simulation_parser::Load_simulation_data_into_scene(const std::filesystem::path & path, eruptor::scene::Scene & base_scene)
{
    Load_file_to_buffor(path);
    if(error_happen)
    {
        return std::unexpected{error_message};
    }

    Simulation_scene sim_scene{ base_scene };

    std::string_view buffor_view = buffor;
    size_t pos{};
    while(pos < buffor_view.size())
    {
        size_t end = buffor_view.find('\n', pos);
        if(end == std::string_view::npos) end = buffor_view.size();
        line_count++;

        std::string_view line = buffor_view.substr(pos, end - pos);
        if(line.size() == 0) //If line empty = skip.
        {
            pos = end + 1;
            continue;
        }

        if(line_mode == Line_mode::NONE) //If none mode select it.
        {
            switch(line[0])
            {
                case '#':                      //Comment
                    pos = end + 1;
                    continue;
                case '<':
                    line_mode = Line_mode::OBJECT_DECLARATION;
                    break;
                default:
                    line_mode = Line_mode::NONE;
                    break;
            }
        }

        Parse_line(line, sim_scene);
        if(error_happen)
        {
            return std::unexpected{error_message};
        }

        pos = end + 1;
    }

    uint32_t max_entity_id = 0;
    for(auto & [alias, id] : sim_scene.objects_aliases)
    {
        if(alias.starts_with("Blob_"))
        {
            uint32_t n = std::stoul(alias.substr(5));
            max_entity_id = std::max(max_entity_id, n + 1);
        }
    }
    sim_scene.next_entity_alias_id = max_entity_id;

    uint32_t max_food_id = 0;
    for(auto & [alias, id] : sim_scene.objects_aliases)
    {
        if(alias.starts_with("Blob_"))
        {
            uint32_t n = std::stoul(alias.substr(5));
            max_food_id = std::max(max_food_id, n + 1);
        }
    }
    sim_scene.next_food_alias_id = max_food_id;


    return sim_scene;
}

void ovum::Simulation_parser::Load_file_to_buffor(const std::filesystem::path & path)
{
    std::ifstream file{path};
    if(!file)
    {
        error_happen = true;
        error_message = error_file_load;
        return;
    }

    std::ostringstream ss{};
    ss << file.rdbuf();

    buffor = ss.str();

    error_happen = false;
    error_message = "";
    line_mode = Line_mode::VERSION_CHECK;
    line_count = 0;
}

void ovum::Simulation_parser::Parse_line(std::string_view line, Simulation_scene & scene)
{
    if(!line.empty() && line.back() == '\r')
    {
        line.remove_suffix(1);
    }

    switch(line_mode)
    {
        case Line_mode::VERSION_CHECK:
        {
            size_t colon_pos = line.find_first_of(":");

            if(colon_pos == std::string_view::npos || line.substr(0, colon_pos) != "Version")
            {
                std::println(std::cerr, "SIMULATION PARSER: Missing version declaration in top of the file: {}", line);

                error_happen = true;
                error_message = error_text_parsing;
                return;
            }

            size_t version_start = line.find_first_not_of(" ", colon_pos + 1);
            if(version_start == std::string_view::npos)
            {
                std::println(std::cerr, "SIMULATION PARSER: Missing version in version declaration: {}", line);

                error_happen = true;
                error_message = error_text_parsing;
                return;
            }

            auto version = line.substr(version_start);
            if(version == "1_0")
            {
                file_version = Version::V1_0;
            }
            else
            {
                std::println(std::cerr, "SIMULATION PARSER: Unknown version in version declaration: {}", line);

                error_happen = true;
                error_message = error_text_parsing;
                return;
            }

            line_mode = Line_mode::NONE;
            break;
        }
        case Line_mode::OBJECT_DECLARATION:
        {
            if(line.back() != '>')
            {
                std::println(std::cerr, "SIMULATION PARSER: Missing closing object declaration bracket in line {}: {}", line_count, line);

                error_happen = true;
                error_message = error_text_parsing;
                return;
            }

            size_t colon_pos = line.find_first_of(":");
            if(colon_pos == std::string_view::npos)
            {
                std::println(std::cerr, "SIMULATION PARSER: Missing colon in object declaration in line {}: {}", line_count, line);

                error_happen = true;
                error_message = error_text_parsing;
                return;
            }

            size_t last_object_name_pos = line.find_first_of(": ",1);

            std::string_view object_name = line.substr(1, last_object_name_pos - 1);
            uint32_t object_id{};

            auto object_name_it = scene.objects_aliases.find( std::string{object_name} );
            if(object_name_it == scene.objects_aliases.end())
            {
                std::println(std::cerr, "SIMULATION PARSER: Unknown object name in object declaration in line {}: {}", line_count, line);

                error_happen = true;
                error_message = error_text_parsing;
                return;
            }
            object_id = object_name_it->second;

            size_t object_type_start_pos = line.find_first_not_of(": ", colon_pos);
            size_t object_type_end_pos = line.find_last_not_of(" >");

            std::string_view object_type = line.substr(object_type_start_pos, object_type_end_pos - object_type_start_pos + 1);
            if(object_type == "Entity")
            {
                current_parsed_entity_data.render_object_id = object_id;

                line_mode = Line_mode::ENTITY_SPEED_DECLARATION;
            }
            else if(object_type == "Food")
            {
                current_parsed_food_data.render_object_id = object_id;

                scene.food.push_back(current_parsed_food_data);
                line_mode = Line_mode::NONE;
            }
            else if(object_type == "Light_source")
            {
                current_parsed_light_source_data.render_object_id = object_id;
                scene.render_objects[ object_id ].shading_type = eruptor::scene::Shading_type::LIGHT_CASTER;

                scene.light_sources.push_back(current_parsed_light_source_data);
                line_mode = Line_mode::NONE;
            }
            else if(object_type == "Floor")
            {
                scene.floor = object_id;
                line_mode = Line_mode::NONE;
            }
            else
            {
                std::println(std::cerr, "SIMULATION PARSER: Unknown object type in object declaration in line {}: {}", line_count, line);

                error_happen = true;
                error_message = error_text_parsing;
                return;
            }

            break;
        }
        case Line_mode::ENTITY_SPEED_DECLARATION:
        {
            size_t colon_pos = line.find_first_of(":");
            if(colon_pos == std::string_view::npos)
            {
                std::println(std::cerr, "SIMULATION PARSER: Missing colon in object speed declaration in line {}: {}", line_count, line);

                error_happen = true;
                error_message = error_text_parsing;
                return;
            }

            if(line.substr(0, colon_pos) != "Speed")
            {
                std::println(std::cerr, "SIMULATION PARSER: Missing speed declaration in object declaration in line {}: {}", line_count, line);

                error_happen = true;
                error_message = error_text_parsing;
                return;
            }

            size_t speed_amount_start_pos = line.find_first_not_of(": ", colon_pos + 1);
            if(speed_amount_start_pos == std::string_view::npos)
            {
                std::println(std::cerr, "SIMULATION PARSER: Missing value in object speed declaration in line {}: {}", line_count, line);

                error_happen = true;
                error_message = error_text_parsing;
                return;
            }

            std::string_view speed_amount = line.substr(speed_amount_start_pos);

            auto [ptr, error_code] = std::from_chars(speed_amount.data(), speed_amount.data() + speed_amount.size(), current_parsed_entity_data.speed);
            if(error_code != std::errc{} || ptr != speed_amount.data() + speed_amount.size())
            {
                std::println(std::cerr, "SIMULATION PARSER: Failed to parse speed amount '{}' in line {}: {}", speed_amount, line_count, line);

                error_happen = true;
                error_message = error_text_parsing;
                return;
            }

            scene.entieties.push_back( current_parsed_entity_data );
            physic_manager->Add_hitbox(1, scene.entieties.back().render_object_id, scene);
            auto & sense_hitbox = physic_manager->Get_hitbox_data(1, scene.entieties.back().render_object_id);
            auto scale = scene.render_objects[ scene.entieties.back().render_object_id ].Get_scale();
            auto sense = scene.entieties.back().sense;
            sense_hitbox.Set_individual_scale( {scale.x + sense, scale.y + sense, scale.z + sense } );
            sense_hitbox.Set_is_active( true );

            line_mode = Line_mode::NONE;
            break;
        }
        case Line_mode::NONE:
            std::println(std::cerr, "SIMULATION PARSER: Unknown syntax in line {}: {}", line_count, line);

            error_happen = true;
            error_message = error_text_parsing;
            break;
    }
}
