#pragma once
#include <glm/glm.hpp>
#include <RayTriangleIntersection.h>
#include <vector>

namespace Raytrace {
	RayTriangleIntersection getClosestValidIntersection(glm::vec3 position, glm::vec3 direction, const std::vector<ModelTriangle>& objects);
}