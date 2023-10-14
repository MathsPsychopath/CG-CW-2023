#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <array>
#include "TextureMap.h"

struct InterpolatedTriangle {
	std::vector<float> topLeft;
	std::vector<float> topRight;
	std::vector<float> leftBottom;
	std::vector<float> rightBottom;
};

struct BarycentricCoordinates {
	float A;
	float B;
	float C;
};

class Interpolate {
private:
	static std::array<glm::vec2, 4> projectLeftVertex(const std::array<glm::vec2, 3>& sortedVertices);
public:
	static std::vector<float> singleFloat(float from, float to, int numberOfValues);
	static std::vector<glm::vec3> threeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValues);

	/// <summary>
	/// This takes the vertices in sorted order by Y-value, and does interpolation
	/// </summary>
	/// <param name="sortedVertices">vertices sorted by y-value lowest to highest</param>
	/// <returns>Struct pointing to interpolated values</returns>
	static InterpolatedTriangle triangle(const std::array<glm::vec2, 3>& sortedVertices);

	static BarycentricCoordinates barycentric(const std::array<glm::vec2, 3>& sortedVertices, glm::vec2 encodedVertex);
};