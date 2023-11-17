#pragma once
#include <glm/vec3.hpp>
#include <Colour.h>

struct GouraudVertex {
	glm::vec3 position{};
	glm::vec3 normal;
	float ambient;
	float proximity;
	float incidental;
	float specular;

	Colour color;

	operator glm::vec3();

	GouraudVertex();
	GouraudVertex(glm::vec3 position);
};