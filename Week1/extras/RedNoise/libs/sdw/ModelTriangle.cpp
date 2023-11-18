#include "ModelTriangle.h"
#include <utility>

ModelTriangle::ModelTriangle() = default;

ModelTriangle::ModelTriangle(int v1Index, int v2Index, int v3Index, Colour trigColour) :
		vertices({v1Index, v2Index, v3Index}), texturePoints(), colour(std::move(trigColour)), normal() {}


std::ostream &operator<<(std::ostream &os, const ModelTriangle &triangle) {
	os << "(" << triangle.vertices[0] << ", " << triangle.vertices[0] << ", " << triangle.vertices[0] << ")\n";
	os << "(" << triangle.vertices[1] << ", " << triangle.vertices[1] << ", " << triangle.vertices[1] << ")\n";
	os << "(" << triangle.vertices[2] << ", " << triangle.vertices[2] << ", " << triangle.vertices[2] << ")\n";
	return os;
}
