#pragma once
#include <DrawingWindow.h>
#include <CanvasPoint.h>
#include <Colour.h>
#include "Camera.h"
#include <CanvasTriangle.h>
#include <unordered_map>

namespace Wireframe {
	void drawLine(DrawingWindow& window, CanvasPoint start, CanvasPoint end, Colour color);

	void drawStrokedTriangle(DrawingWindow& window, CanvasTriangle triangle, Colour color);

	void drawPointCloud(DrawingWindow& window, Camera& camera, std::unordered_map<std::string, glm::vec3> loadedVertices);

	CanvasPoint canvasIntersection(Camera& camera, glm::vec3 vertexPosition, float focalLength, const glm::mat3& viewMatrix = glm::mat3(1.0));

}