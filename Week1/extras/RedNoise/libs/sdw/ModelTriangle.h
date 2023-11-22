#pragma once

#include <glm/glm.hpp>
#include <string>
#include <array>
#include "Colour.h"
#include "TexturePoint.h"
#include "GouraudVertex.h"

struct ModelTriangle {
	std::array<int, 3> vertices{};
	std::array<int, 3> texturePoints{};
	Colour colour{};
	glm::vec3 normal{};
	std::pair<glm::vec3, glm::vec3> boundingMinMax;

	ModelTriangle();
	ModelTriangle(int v1Index, int v2Index, int v3Index, Colour trigColour);
	ModelTriangle(int v1Index, int v2Index, int v3Index, int t1Index, int t2Index, int t3Index);
	friend std::ostream &operator<<(std::ostream &os, const ModelTriangle &triangle);
};
