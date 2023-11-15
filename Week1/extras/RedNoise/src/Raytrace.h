#pragma once
#include <glm/glm.hpp>
#include <RayTriangleIntersection.h>
#include <vector>
#include <CanvasPoint.h>
#include "Camera.h"
#include <DrawingWindow.h>

namespace Raytrace {
	RayTriangleIntersection getClosestValidIntersection(glm::vec3 position, glm::vec3 direction, const std::vector<ModelTriangle>& objects, int excludeID = -1, float lightDistance = std::numeric_limits<float>::max());

	// inverse of getCanvasIntersection
	glm::vec3 getCanvasPosition(Camera& camera, CanvasPoint position, glm::mat3 inverseViewMatrix);

	// draws black when no direct light
	void drawHardShadows(DrawingWindow& window, CanvasPoint point, Colour color, bool hasDirectLight);

	// draws proximal lighting
	//void drawProximityShadows(DrawingWindow& window, CanvasPoint point, Colour color, )
}