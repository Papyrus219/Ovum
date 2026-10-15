#include <Ovum/stat_chart_manager.hpp>
#include <map>

using namespace ovum;

void ovum::Stat_chart_manager::Setup(Simulation_settings & sim_settings)
{
    if(sim_settings.is_speed_evo_enabled)
    {
        if(sim_settings.is_size_evo_enabled)
        {
            if(sim_settings.is_sense_evo_enabled)
            {
                graph_mode = Graph_mode::SPEED_SIZE_SENSE;
            }
            else
            {
                graph_mode = Graph_mode::SPEED_SIZE;
            }
        }
        else if(sim_settings.is_sense_evo_enabled)
        {
            graph_mode = Graph_mode::SPEED_SENSE;
        }
        else
        {
            graph_mode = Graph_mode::SPEED;
        }
    }
    else if(sim_settings.is_size_evo_enabled)
    {
        if(sim_settings.is_sense_evo_enabled)
        {
            graph_mode = Graph_mode::SIZE_SENSE;
        }
        else
        {
            graph_mode = Graph_mode::SIZE;
        }
    }
    else if(sim_settings.is_sense_evo_enabled)
    {
        graph_mode = Graph_mode::SENSE;
    }
    else
    {
        graph_mode = Graph_mode::NONE;
    }

    auto Set_stat_ranges = [&](Stat_range x, Stat_range y, Stat_range z = NONE_RANGE, bool use_z = false)
    {
        gp_comm_stats.Set_x_axis_range(x.min, x.max);
        if(y.max > y.min)
        {
            gp_comm_stats.Set_y_axis_range(y.min, y.max);
        }
        if(use_z)
        {
            gp_comm_stats.Set_z_axis_range(z.min, z.max);
        }
    };

    switch(graph_mode)
    {
        case Graph_mode::SPEED_SIZE_SENSE:
        {
            gp_comm_stats.Enable_3d_points("STATS", 1, true);
            gp_comm_stats.Set_x_axis_title("Speed");
            gp_comm_stats.Set_y_axis_title("Size");
            gp_comm_stats.Set_z_axis_title("Sense");
            Set_stat_ranges(SPEED_RANGE, SIZE_RANGE, SENSE_RANGE, true);
            break;
        }
        case Graph_mode::SPEED_SIZE:
        {
            gp_comm_stats.Enable_2d_points("STATS", true);
            gp_comm_stats.Set_x_axis_title("Speed");
            gp_comm_stats.Set_y_axis_title("Size");
            Set_stat_ranges(SPEED_RANGE, SIZE_RANGE);
            break;
        }
        case Graph_mode::SPEED_SENSE:
        {
            gp_comm_stats.Enable_2d_points("STATS", true);
            gp_comm_stats.Set_x_axis_title("Speed");
            Set_stat_ranges(SPEED_RANGE, SENSE_RANGE);
            break;
        }
        case Graph_mode::SIZE_SENSE:
        {
            gp_comm_stats.Enable_2d_points("STATS", true);
            gp_comm_stats.Set_x_axis_title("Size");
            gp_comm_stats.Set_y_axis_title("Sense");
            Set_stat_ranges(SIZE_RANGE, SENSE_RANGE);
            break;
        }
        case Graph_mode::SPEED:
        {
            gp_comm_stats.Enable_2d_bars("SPEED distribution", true);
            gp_comm_stats.Set_x_axis_title("Speed");
            gp_comm_stats.Set_y_axis_title("Entities");
            Set_stat_ranges(SPEED_RANGE, NONE_RANGE);
            break;
        }
        case Graph_mode::SIZE:
        {
            gp_comm_stats.Enable_2d_bars("SIZE distribution", true);
            gp_comm_stats.Set_x_axis_title("Size");
            gp_comm_stats.Set_y_axis_title("Entities");
            Set_stat_ranges(SIZE_RANGE, NONE_RANGE);
            break;
        }
        case Graph_mode::SENSE:
        {
            gp_comm_stats.Enable_2d_bars("SENSE distribution", true);
            gp_comm_stats.Set_x_axis_title("Sense");
            gp_comm_stats.Set_y_axis_title("Entities");
            Set_stat_ranges(SENSE_RANGE, NONE_RANGE);
            break;
        }
        case Graph_mode::NONE:
        default:
            break;
    }
}

void ovum::Stat_chart_manager::Update(std::span<const Entiety_data> entities)
{
    if(graph_mode == Graph_mode::NONE)
    {
        return;
    }

    if(entities.empty())
    {
        return;
    }

    switch(graph_mode)
    {
        case Graph_mode::SPEED_SIZE_SENSE:
        {
            Draw_colored_points(entities, [](const Entiety_data & e){ return glm::vec3{e.speed, e.size, e.sense}; }, true, SPEED_RANGE, SIZE_RANGE, SENSE_RANGE);
            break;
        }
        case Graph_mode::SPEED_SIZE:
        {
            Draw_colored_points(entities, [](const Entiety_data & e){ return glm::vec3{e.speed, e.size, 0.0f}; }, false, SPEED_RANGE, SIZE_RANGE, NONE_RANGE);
            break;
        }
        case Graph_mode::SPEED_SENSE:
        {
            Draw_colored_points(entities, [](const Entiety_data & e){ return glm::vec3{e.speed, e.sense, 0.0f}; }, false, SPEED_RANGE, SENSE_RANGE, NONE_RANGE);
            break;
        }
        case Graph_mode::SIZE_SENSE:
        {
            Draw_colored_points(entities, [](const Entiety_data & e){ return glm::vec3{e.size, e.sense, 0.0f}; }, false, SIZE_RANGE, SENSE_RANGE, NONE_RANGE);
            break;
        }
        case Graph_mode::SPEED:
            Draw_histogram(entities, &Entiety_data::speed);
            break;
        case Graph_mode::SIZE:
            Draw_histogram(entities, &Entiety_data::size);
            break;
        case Graph_mode::SENSE:
            Draw_histogram(entities, &Entiety_data::sense);
            break;
        case Graph_mode::NONE:
        default:
            break;
    }
}

void ovum::Stat_chart_manager::Draw_histogram(std::span<const Entiety_data> entities, float Entiety_data::* member)
{
    float min = entities.front().*member;
    float max = min;

    for(auto & entity : entities)
    {
        const float v = entity.*member;
        min = std::min(min, v);
        max = std::max(max, v);
    }

    const float range = (max > min) ? (max - min) : 1.0f;
    const float bin_width = range / 20;

    std::vector<uint32_t> bins(20, 0);

    for(auto & entity : entities)
    {
        const float v = entity.*member;
        int index = static_cast<std::size_t>((v - min) / bin_width);
        index = std::min(index, 20 - 1);
        bins[index]++;
    }

    gp_comm_stats.Begin_frame();
    for(std::size_t i = 0; i < 20; i++)
    {
        gp_comm_stats.Stage_data(glm::vec2{ min + (i + 0.5f) * bin_width, static_cast<float>(bins[i])}, 1, 0);
    }
    gp_comm_stats.End_frame();
}

template<typename T>
void ovum::Stat_chart_manager::Draw_colored_points(std::span<const Entiety_data> entities, T Get_point, bool is_3d, Stat_range rx, Stat_range ry, Stat_range rz)
{
    std::map<std::array<int, 3>, Point_bucket> buckets{};

    for(auto & entity : entities)
    {
        const glm::vec3 point = Get_point(entity);

        const std::array<int, 3> key{static_cast<int>(std::round(point.x / BUCKET_MARGIN)), static_cast<int>(std::round(point.y / BUCKET_MARGIN)), static_cast<int>(std::round(point.z / BUCKET_MARGIN))};

        auto & bucket = buckets[key];
        if(bucket.count == 0)
        {
            bucket.pos = point;
        }
        bucket.count++;
    }

    gp_comm_stats.Begin_frame();
    for(auto & [key, bucket] : buckets)
    {
        glm::vec3 n{
            (bucket.pos.x - rx.min) / (rx.max - rx.min),
            (bucket.pos.y - ry.min) / (ry.max - ry.min),
            (bucket.pos.z - rz.min) / (rz.max - rz.min)
        };

        if(!is_3d)
        {
            n.x = COLOR_FLOOR + (1.0f - COLOR_FLOOR) * glm::clamp(n.x, 0.0f, 1.0f);
            n.y = COLOR_FLOOR + (1.0f - COLOR_FLOOR) * glm::clamp(n.y, 0.0f, 1.0f);
            n.z = COLOR_FLOOR;
        }

        if(!is_3d)
        {
            n.x = COLOR_FLOOR + (1.0f - COLOR_FLOOR) * glm::clamp(n.x, 0.0f, 1.0f);
            n.y = COLOR_FLOOR + (1.0f - COLOR_FLOOR) * glm::clamp(n.y, 0.0f, 1.0f);
            n.z = COLOR_FLOOR;
        }

        const float point_size = std::min(1.0f + 0.5f * (bucket.count - 1), MAX_POINT_SIZE);
        const uint32_t rgb = Pack_rgb(n);

        if(is_3d)
        {
            gp_comm_stats.Stage_data(bucket.pos, point_size, rgb);
        }
        else
        {
            gp_comm_stats.Stage_data(glm::vec2{bucket.pos.x, bucket.pos.y}, point_size, rgb);
        }
    }
    gp_comm_stats.End_frame();
}


uint32_t Stat_chart_manager::Pack_rgb(glm::vec3 n)
{
    n = glm::clamp(n, 0.0f, 1.0f);

    return (static_cast<uint32_t>(n.r * 255) << 16) |
    (static_cast<uint32_t>(n.g * 255) << 8)  |
    static_cast<uint32_t>(n.b * 255);
}
