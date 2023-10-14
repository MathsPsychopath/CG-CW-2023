#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <array>

struct InterpolatedTriangle {
	std::vector<float> topLeftXValues;
	std::vector<float> topRightXValues;
	std::vector<float> leftBottomXValues;
	std::vector<float> rightBottomXValues;
};

class Interpolate {
private:
	static std::vector<float> singleFloat(float from, float to, int numberOfValues);
public:
	static std::vector<glm::vec3> threeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValues);

	/// <summary>
	/// This takes the vertices in sorted order by Y-value, and does interpolation
	/// </summary>
	/// <param name="v1">Top-most y-value vertex</param>
	/// <param name="v2">Middle vertex</param>
	/// <param name="v3">Bottom-most y-value vertex</param>
	/// <returns>Struct pointing to interpolated values</returns>
	static InterpolatedTriangle triangle(const std::array<glm::vec2, 3>& sortedVertices);
};