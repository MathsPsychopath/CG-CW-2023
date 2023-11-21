#pragma once
#include <glm/glm.hpp>
#include <RayTriangleIntersection.h>
#include <CanvasPoint.h>
#include "Camera.h"
#include <DrawingWindow.h>
#include <PolygonData.h>
#include <TextureMap.h>
#include "Lighting.h"

namespace Raytrace {
	RayTriangleIntersection getClosestValidIntersection(glm::vec3 position, glm::vec3 direction, PolygonData& objects, int excludeID = -1, float lightDistance = std::numeric_limits<float>::max());

	// inverse of getCanvasIntersection
	glm::vec3 getCanvasPosition(Camera& camera, CanvasPoint position, glm::mat3 inverseViewMatrix);

	void useGouraud(DrawingWindow& window, Camera& camera, PolygonData& objects, TextureMap& textures, glm::vec3 lightPosition);

	void usePhong(DrawingWindow& window, Camera& camera, PolygonData& objects, glm::vec3 lightPosition);

	void preprocessGouraud(PolygonData& objects, glm::vec3 lightPosition, glm::vec3 cameraPosition, bool& hasParametersChanged);

}