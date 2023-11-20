#pragma once
#include <glm/vec3.hpp>
#include <Colour.h>

struct GouraudVertex {
	glm::vec3 position{};
	glm::vec3 normal;
	Colour ambient;
	float proximity;
	float incidental;
	Colour specular;
	Colour diffuse;

	Colour originalColor;
	Colour renderedColor;

	operator glm::vec3();

	GouraudVertex();
	GouraudVertex(glm::vec3 position, Colour color);
	friend std::ostream& operator<<(std::ostream& os, const GouraudVertex& gv);
};