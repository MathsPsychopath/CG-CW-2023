#include "PolygonData.h"

PolygonData::PolygonData() = default;

PolygonData::PolygonData(std::unordered_map<int, std::set<int>> vertexToTriangles,
	std::vector<ModelTriangle> loadedTriangles,
	std::vector<GouraudVertex> loadedVertices) :
	vertexToTriangles(vertexToTriangles), loadedTriangles(loadedTriangles), loadedVertices(loadedVertices) {}

GouraudVertex& PolygonData::getTriangleVertex(int triangleIndex, int triangleVertexIndex) {
	return this->loadedVertices[this->loadedTriangles[triangleIndex].vertices[triangleVertexIndex]];
}

glm::vec3 PolygonData::getTriangleVertexPosition(int triangleIndex, int triangleVertexIndex) {
	return this->loadedVertices[this->loadedTriangles[triangleIndex].vertices[triangleVertexIndex]].position;
}
