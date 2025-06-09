#pragma once
#include "glm/vec3.hpp"

namespace VizUtils
{
    inline glm::vec3 toGL(glm::vec3 const& vec)
    {
        return glm::vec3(vec.x, vec.z, vec.y);
        // return vec;
    }
} // namespace VizUtils