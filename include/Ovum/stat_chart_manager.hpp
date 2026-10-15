#ifndef OVUM_STAT_CHART_MANAGER_HPP
#define OVUM_STAT_CHART_MANAGER_HPP

#include <Ovum/gp_communicator.hpp>
#include <Ovum/simulation_settings.hpp>
#include <glm/glm.hpp>
#include <span>

namespace ovum
{

class Stat_chart_manager
{
public:
    void Setup(Simulation_settings & sim_settings);

    void Update(std::span<const Entiety_data> entities);

private:
    struct Point_bucket
    {
        glm::vec3 pos{};
        uint32_t count{};
    };

    struct Stat_range
    {
        float min;
        float max;
    };

    void Draw_histogram(std::span<const Entiety_data> entities, float Entiety_data::* member);
    template<typename T>
    void Draw_colored_points(std::span<const Entiety_data> entities, T Get_point, bool is_3d, Stat_range rx, Stat_range ry, Stat_range rz);

    uint32_t Pack_rgb(glm::vec3 n);

    ovum::GP_communicator gp_comm_stats{};

    static constexpr float BUCKET_MARGIN{0.1f};
    static constexpr float MAX_POINT_SIZE{6.0f};

    static constexpr Stat_range SPEED_RANGE{0, 6};
    static constexpr Stat_range SIZE_RANGE{0, 6};
    static constexpr Stat_range SENSE_RANGE{0, 12};
    static constexpr Stat_range NONE_RANGE{0, 1};

    static constexpr float COLOR_FLOOR{0.3f};

    enum class Graph_mode
    {
        NONE,
        SPEED,
        SIZE,
        SENSE,
        SPEED_SIZE,
        SPEED_SENSE,
        SIZE_SENSE,
        SPEED_SIZE_SENSE,
    } graph_mode;
};

}

#endif // OVUM_STAT_CHART_MANAGER_HPP
