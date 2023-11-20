#include "PolygonData.h"

PolygonData::PolygonData() = default;

PolygonData::PolygonData(std::unordered_map<int, std::set<int>> vertexToTriangles,
	std::vector<ModelTriangle> loadedTriangles,
	std::vector<GouraudVertex> loadedVertices,
	std::vector<TexturePoint> loadedTextures) :
	vertexToTriangles(vertexToTriangles), loadedTriangles(loadedTriangles), loadedVertices(loadedVertices), loadedTextures(loadedTextures) {}

GouraudVertex& PolygonData::getTriangleVertex(int triangleIndex, int triangleVertexIndex) {
	return this->loadedVertices[this->loadedTriangles[triangleIndex].vertices[triangleVertexIndex]];
}

glm::vec3 PolygonData::getTriangleVertexPosition(int triangleIndex, int triangleVertexIndex) {
	return this->loadedVertices[this->loadedTriangles[triangleIndex].vertices[triangleVertexIndex]].position;
}

std::array<glm::vec2, 3> PolygonData::getTextureVertices(int triangleIndex) {
	std::array<glm::vec2, 3> textureVertices;
	int i = 0;
	for (int textureIndex : this->loadedTriangles[triangleIndex].texturePoints) {
		textureVertices[i] = glm::vec2(this->loadedTextures[textureIndex].x, this->loadedTextures[textureIndex].y);
		i++;
	}
	return textureVertices;
}
