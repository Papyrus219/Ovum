#include <Ovum/gp_communicator.hpp>
#include <stdexcept>

using namespace ovum;

ovum::GP_communicator::GP_communicator(bool is_persist)
{
    if(is_persist)
    {
         gnuplot = popen("gnuplot -persist", "w");
    }
    else
    {
        gnuplot = popen("gnuplot", "w");
    }

    if(!gnuplot)
    {
        throw std::runtime_error{"Failed to open stream with gnuplot,"};
    }
}

void ovum::GP_communicator::Begin_frame()
{
    if(mode == GP_mode::MODE_2D_POINT)
    {
        fprintf(gnuplot, "plot '-' with points pointtype 7\n");
    }
    else if(mode == GP_mode::Mode_2D_BARS)
    {
        fprintf(gnuplot, "plot '-' with boxes\n");
    }
    else if(mode == GP_mode::MODE_3D_POINT)
    {
        fprintf(gnuplot, "splot '-' with points pointtype 7\n");
    }
    else
    {
        throw(std::runtime_error{"Gnuplot comunicator hasn't set mode of use in moment of use."});
    }
}

void ovum::GP_communicator::End_frame()
{
    fprintf(gnuplot, "e\n");
    fflush(gnuplot);
}

void ovum::GP_communicator::Enable_2d_points(const std::string & title)
{
    fprintf(gnuplot, "set title '%s'\n",  title.c_str());

    mode = GP_mode::MODE_2D_POINT;
}

void ovum::GP_communicator::Enable_2d_bars(const std::string& title, bool enable_floting_points)
{
    fprintf(gnuplot, "set title '%s'\n", title.c_str());
    fprintf(gnuplot, "set boxwidth 0.08\n");
    fprintf(gnuplot, "set style fill solid 0.6 border -1\n");

    if(!enable_floting_points)
    {
        fprintf(gnuplot, "set xtics 1\n");
        fprintf(gnuplot, "set ytics 1\n");
    }

    mode = GP_mode::Mode_2D_BARS;
}

void ovum::GP_communicator::Enable_3d_points(const std::string & title, uint32_t size)
{
    fprintf(gnuplot, "set title '%s'\n",  title.c_str());
    fprintf(gnuplot, "set pointsize %u \n", size);

    fprintf(gnuplot, "set grid xtics ytics ztics\n");
    fprintf(gnuplot, "set xyplane at 0\n");
    fprintf(gnuplot, "set border 4095\n");
    fprintf(gnuplot, "set view 60,30\n");

    mode = GP_mode::MODE_3D_POINT;
}

void ovum::GP_communicator::Set_title(const std::string & title)
{
    fprintf(gnuplot, "set title '%s'\n",  title.c_str());
}

void ovum::GP_communicator::Set_x_axis_range(float min, float max)
{
    fprintf(gnuplot, "set xrange [%f:%f]\n", min, max);
}

void ovum::GP_communicator::Set_y_axis_range(float min, float max)
{
    fprintf(gnuplot, "set yrange [%f:%f]\n", min, max);
}

void ovum::GP_communicator::Set_z_axis_range(float min, float max)
{
    fprintf(gnuplot, "set zrange [%f:%f]\n", min, max);
}

void ovum::GP_communicator::Set_x_axis_title(const std::string & title)
{
    fprintf(gnuplot, "set xlabel '%s'\n", title.c_str());
}

void ovum::GP_communicator::Set_y_axis_title(const std::string & title)
{
    fprintf(gnuplot, "set ylabel '%s'\n", title.c_str());
}

void ovum::GP_communicator::Set_z_axis_title(const std::string & title)
{
    fprintf(gnuplot, "set zlabel '%s'\n", title.c_str());
}

void ovum::GP_communicator::Stage_data(glm::vec3 vec3)
{
    fprintf(gnuplot, "%f %f %f\n", vec3.x, vec3.y, vec3.z);
}

void ovum::GP_communicator::Stage_data(glm::vec2 vec2)
{
    fprintf(gnuplot, "%f %f\n", vec2.x, vec2.y);
}

void ovum::GP_communicator::Stage_data(float x)
{
    fprintf(gnuplot, "%f\n", x);
}

ovum::GP_communicator::~GP_communicator()
{
    if(gnuplot)
    {
        pclose(gnuplot);
    }
}
