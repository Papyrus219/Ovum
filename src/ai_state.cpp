#include <Ovum/ai_state.hpp>

void ovum::Ai_data::Add_dirr_of_intrest(glm::vec3 from, glm::vec3 to, float value, float fallback)
{
    glm::vec3 delta = to - from;
    delta.y = 0;

    float dis_sqrt = delta.x * delta.x* + delta.z * delta.z;
    if(dis_sqrt < 1e-6f)
    {
        return;
    }

    float dis = std::sqrt( dis_sqrt );
    float fall = dis_sqrt;
    for(int i{}; i < fallback;i++)
    {
        fall *= dis;
    }

    desire_dirr += (delta * value) / fall;
}
