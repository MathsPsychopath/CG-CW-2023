#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <CanvasPoint.h>
#include "Camera.h"
#include <array>
#include <DrawingWindow.h>
#include <CanvasTriangle.h>
#include <Colour.h>
#include <TextureMap.h>

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

namespace Rasterize {
	// interpolates all the values between 2 vec3s
	std::vector<glm::vec3> threeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValues);

	// returns the 4 x-value interpolated edges of the divided triangle.
	InterpolatedTriangle triangle(const std::array<CanvasPoint, 3>& sortedVertices);

	// computes the canvas intersection of a triangle point wrt camera position
	CanvasPoint canvasIntersection(Camera& camera, glm::vec3 vertexPosition, float focalLength, const glm::mat3& viewMatrix = glm::mat3(1.0));

	// rasterizes a solid color triangle
	void drawRasterizedTriangle(DrawingWindow& window, CanvasTriangle triangle, Colour color, std::vector<std::vector<float>>& zDepth);

	// rasterizes a textured triangle
	void drawRasterizedTriangle(DrawingWindow& window, CanvasTriangle triangle, TextureMap textures, std::vector<std::vector<float>>& zDepth);

	BarycentricCoordinates barycentric(const std::array<CanvasPoint, 3>& sortedVertices, glm::vec2 encodedVertex);

}