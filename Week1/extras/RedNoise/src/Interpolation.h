#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <array>
#include "TextureMap.h"
#include <CanvasPoint.h>
#include "Constants.h"
#include <CanvasTriangle.h>

struct InterpolatedTriangle {
	std::vector<float> topLeft;
	std::vector<float> topRight;
	std::vector<float> leftBottom;
	std::vector<float> rightBottom;
};

namespace Interpolate {
	std::vector<float> singleFloat(float from, float to, int numberOfValues);
	std::vector<glm::vec3> threeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValues);

	/// <summary>
	/// This takes the vertices in sorted order by Y-value, and does interpolation
	/// </summary>
	/// <param name="sortedVertices">vertices sorted by y-value lowest to highest</param>
	/// <returns>Struct pointing to interpolated values</returns>
	InterpolatedTriangle triangle(const std::array<CanvasPoint, 3>& sortedVertices);

	CanvasPoint canvasIntersection(glm::vec3 cameraPosition, glm::vec3 vertexPosition, float focalLength);

	void zDepth(CanvasTriangle interpolateTarget, std::vector<std::vector<float>>& depthMap);
};

