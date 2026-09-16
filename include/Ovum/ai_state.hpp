#ifndef OVUM_AI_STATE_HPP
#define OVUM_AI_STATE_HPP

#include <glm/glm.hpp>

namespace ovum
{

enum class Ai_state
{
    RESTING,
    HUNTING,
    RETURN,
    DEAD
};

struct Ai_data
{
    void Add_dirr_of_intrest(glm::vec3 from, glm::vec3 to, float value, float fallback);
    glm::vec3 desire_dirr{};

    float desire_y_rot{};
    float curr_y_rot{};

    float time_elapsed{};

    Ai_state state{};

    bool is_desire_rot{true};
};

}

#endif //OVUM_AI_STATE_HPP
