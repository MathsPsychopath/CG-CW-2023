#pragma once
#include <glm/glm.hpp>
#include <RayTriangleIntersection.h>
#include <CanvasPoint.h>
#include "Camera.h"
#include <DrawingWindow.h>
#include <PolygonData.h>
#include <TextureMap.h>
#include "Lighting.h"
#include <thread>
#include <sstream>

namespace Raytrace {

	void preprocessGouraud(PolygonData& objects, glm::vec3& lightPosition, glm::vec3& cameraPosition);

	void renderSegment(glm::vec2 boundY, std::vector<std::vector<uint32_t>>& colorBuffer, PolygonData& objects, Camera& camera, TextureMap& textures, glm::vec3 lightOrigin, std::set<std::string>& hiddenObjects);
}