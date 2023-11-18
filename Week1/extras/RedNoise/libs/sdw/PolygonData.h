#pragma once
#include <vector>
#include <unordered_map>
#include <ModelTriangle.h>
#include <set>

struct PolygonData {
	std::unordered_map<int, std::set<int>> vertexToTriangles;
	std::vector<ModelTriangle> loadedTriangles;
	std::vector<GouraudVertex> loadedVertices;

	PolygonData();
	PolygonData(std::unordered_map<int, std::set<int>> vertexToTriangles,
		std::vector<ModelTriangle> loadedTriangles,
		std::vector<GouraudVertex> loadedVertices);

	GouraudVertex& getTriangleVertex(int triangleIndex, int triangleVertexIndex);

	glm::vec3 getTriangleVertexPosition(int triangleIndex, int triangleVertexIndex);
};
