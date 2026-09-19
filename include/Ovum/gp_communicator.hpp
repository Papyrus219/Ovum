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
    Mode_2D_BARS,
    MODE_3D_POINT,
};

class GP_communicator
{
public:
    GP_communicator(bool is_persist = false);

    void Begin_frame();
    void End_frame();

    void Enable_2d_points(const std::string & title);
    void Enable_2d_bars(const std::string & title, bool enable_floting_points = true);
    void Enable_3d_points(const std::string & title, uint32_t size = 1);

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

    void Stage_data(glm::vec3 vec3);
    void Stage_data(glm::vec2 vec2);
    void Stage_data(float x);

    ~GP_communicator();

private:
    FILE * gnuplot{};
    GP_mode mode{};
};

}

#endif // OVUM_GP_COMMUNICATOR_HPP
