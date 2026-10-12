#ifndef OVUM_GP_COMMUNICATOR_HPP
#define OVUM_GP_COMMUNICATOR_HPP

#include <glm/glm.hpp>
#include <string>
#include <cstdio>


namespace ovum
{

enum class GP_mode
{
    NONE,
    MODE_2D_POINT,
    MODE_2D_POINT_COLOR,
    MODE_2D_BARS,
    MODE_3D_POINT,
    MODE_3D_POINT_COLOR,
};

class GP_communicator
{
public:
    GP_communicator(bool is_persist = false);

    void Begin_frame();
    void End_frame();

    void Enable_2d_points(const std::string & title, bool enable_colors = false);
    void Enable_2d_bars(const std::string & title, bool enable_floting_points = true);
    void Enable_3d_points(const std::string & title, uint32_t size = 1, bool enable_colors = false);

    void Set_title(const std::string & title);

    void Set_x_axis_range(float min, float max);
    void Set_y_axis_range(float min, float max);
    void Set_z_axis_range(float min, float max);

    void Set_x_axis_title(const std::string & title);
    void Set_y_axis_title(const std::string & title);
    void Set_z_axis_title(const std::string & title);

    void Set_x_tics(float step);
    void Set_y_tics(float step);
    void Set_z_tics(float step);

    void Stage_data(glm::vec3 vec3, float point_size, uint32_t rgb);
    void Stage_data(glm::vec2 vec2, float point_size, uint32_t rgb);
    void Stage_data(float x);

    ~GP_communicator();

private:
    FILE * gnuplot{};
    GP_mode mode{};
};

}

#endif // OVUM_GP_COMMUNICATOR_HPP
