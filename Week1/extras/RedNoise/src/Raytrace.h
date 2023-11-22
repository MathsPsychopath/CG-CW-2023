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

	void useGouraud(DrawingWindow& window, Camera& camera, PolygonData& objects, TextureMap& textures, glm::vec3 lightPosition);

	void usePhong(DrawingWindow& window, Camera& camera, PolygonData& objects, glm::vec3 lightPosition);

	void preprocessGouraud(PolygonData& objects, glm::vec3& lightPosition, glm::vec3& cameraPosition, bool& hasParametersChanged);

}