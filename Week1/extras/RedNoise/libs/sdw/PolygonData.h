#pragma once
#include <vector>
#include <unordered_map>
#include "ModelTriangle.h"
#include <set>

struct PolygonData {
	std::unordered_map<int, std::set<int>> vertexToTriangles;
	std::vector<ModelTriangle> loadedTriangles;
	std::vector<GouraudVertex> loadedVertices;
	std::vector<TexturePoint> loadedTextures;
	std::pair<glm::vec3, glm::vec3> sceneBoundingMinMax;

	PolygonData();
	PolygonData(std::unordered_map<int, std::set<int>> vertexToTriangles,
		std::vector<ModelTriangle> loadedTriangles,
		std::vector<GouraudVertex> loadedVertices,
		std::vector<TexturePoint> loadedTextures);

	GouraudVertex& getTriangleVertex(int triangleIndex, int triangleVertexIndex);

	glm::vec3 getTriangleVertexPosition(int triangleIndex, int triangleVertexIndex);

	std::array<glm::vec2, 3> getTextureVertices(int triangleIndex);
};
