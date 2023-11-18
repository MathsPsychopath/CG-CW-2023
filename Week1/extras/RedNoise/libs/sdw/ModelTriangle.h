#pragma once

#include <glm/glm.hpp>
#include <string>
#include <array>
#include "Colour.h"
#include "TexturePoint.h"
#include "GouraudVertex.h"

struct ModelTriangle {
	std::array<int, 3> vertices{};
	std::array<TexturePoint, 3> texturePoints{};
	Colour colour{};
	glm::vec3 normal{};

	ModelTriangle();
	ModelTriangle(int v1Index, int v2Index, int v3Index, Colour trigColour);
	friend std::ostream &operator<<(std::ostream &os, const ModelTriangle &triangle);
};
