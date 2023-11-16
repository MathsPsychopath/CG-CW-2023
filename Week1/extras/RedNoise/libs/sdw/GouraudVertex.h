#pragma once
#include <glm/vec3.hpp>
#include <Colour.h>

struct GouraudVertex {
	glm::vec3 position{};
	glm::vec3 normal;
	Colour color;

	operator glm::vec3();

	GouraudVertex();
};