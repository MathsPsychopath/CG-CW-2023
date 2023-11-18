#pragma once
#include <DrawingWindow.h>
#include <CanvasPoint.h>
#include <Colour.h>
#include "Camera.h"
#include <CanvasTriangle.h>
#include <unordered_map>
#include <ModelTriangle.h>
#include <PolygonData.h>

namespace Wireframe {
	// draws a line between 2 CanvasPoints
	void drawLine(DrawingWindow& window, CanvasPoint start, CanvasPoint end, Colour color);

	// draws a unfilled triangled of specified color
	void drawStrokedTriangle(DrawingWindow& window, CanvasTriangle triangle, Colour color);

	// used for pointcloud representation
	void drawCloudPoints(DrawingWindow& window, Camera& camera, std::vector<CanvasPoint> loadedVertices);

	// computes the canvas intersection of a triangle point wrt camera position
	CanvasPoint canvasIntersection(Camera& camera, glm::vec3 vertexPosition, float focalLength, const glm::mat3& viewMatrix = glm::mat3(1.0));

	// draws wireframe render
	void drawWireframe(DrawingWindow& window, Camera& camera, PolygonData& objects);

}