#include "maths.hpp"
#include <cmath> // For sqrt

namespace mymaths {
    float length(const glm::vec3& v) {
        return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    }

    glm::vec3 normalize(const glm::vec3& v) {
        float l = length(v);
        if (l == 0.0f) return glm::vec3(0.0f); // Avoid division by zero
        return glm::vec3(v.x / l, v.y / l, v.z / l);
    }

    float dot(const glm::vec3& v1, const glm::vec3& v2) {
        return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
    }

    glm::vec3 cross(const glm::vec3& v1, const glm::vec3& v2) {
        return glm::vec3(
            v1.y * v2.z - v1.z * v2.y,
            v1.z * v2.x - v1.x * v2.z,
            v1.x * v2.y - v1.y * v2.x
        );
    }
    // Implement lookAt and perspective later
}