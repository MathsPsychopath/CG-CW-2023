#include "Interpolation.h"

std::array<glm::vec2, 4> Interpolate::projectLeftVertex(const std::array<glm::vec2, 3>& sortedVertices) {

	// Compute projection, which is the linear interpolation formula rearranged for x
	float xProjection = sortedVertices[0].x
		+ ((sortedVertices[1].y - sortedVertices[0].y) / (sortedVertices[2].y - sortedVertices[0].y))
		* (sortedVertices[2].x - sortedVertices[0].x);

	glm::vec2 projectedPoint = glm::vec2(std::round(xProjection), sortedVertices[1].y);

	// decide the left most vertex
	glm::vec2 left = projectedPoint.x < sortedVertices[1].x ? projectedPoint : sortedVertices[1];
	glm::vec2 right = left.x == projectedPoint.x ? sortedVertices[1] : projectedPoint;

	std::array<glm::vec2, 4> output = { sortedVertices[0], left, right, sortedVertices[2] };
	return output;
}

std::vector<float> Interpolate::singleFloat(float from, float to, int numberOfValues) {
	const float increment = (to - from) / (numberOfValues - 1);
	if (from - to == 0) return {from};
	std::vector<float> output = {};
	for (int i = 0; i < numberOfValues; i++) {
		output.push_back(from + i * increment);
	}
	return output;
}

std::vector<glm::vec3> Interpolate::threeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValues) {
	const int indices = numberOfValues - 1;
	glm::vec3 steps((to.x - from.x) / indices, (to.y - from.y) / indices, (to.z - from.z) / indices);
	std::vector<glm::vec3> output = {};
	for (int i = 0; i < numberOfValues; i++) {
		output.push_back(from + (steps * float(i)));
	}
	return output;
}

InterpolatedTriangle Interpolate::triangle(const std::array<glm::vec2, 3>& sortedVertices) {

	std::array<glm::vec2, 4> vertices = projectLeftVertex(sortedVertices);

	InterpolatedTriangle output;

	// interpolate x values for edges
	output.topLeft = singleFloat(vertices[0].x, vertices[1].x, std::abs(vertices[1].y - vertices[0].y));
	output.topRight = singleFloat(vertices[0].x, vertices[2].x, std::abs(vertices[2].y - vertices[0].y));
	output.leftBottom = singleFloat(vertices[1].x, vertices[3].x, std::abs(vertices[3].y - vertices[1].y));
	output.rightBottom = singleFloat(vertices[2].x, vertices[3].x, std::abs(vertices[3].y - vertices[2].y));

	return output;
}

BarycentricCoordinates Interpolate::barycentric(const std::array<glm::vec2, 3>& sortedVertices, glm::vec2 encodedVertex) {
	glm::vec2 top = sortedVertices[0];
	glm::vec2 middle = sortedVertices[1];
	glm::vec2 bottom = sortedVertices[2];
	BarycentricCoordinates output = {};

	// refer to barycentric coordinates math
	float denominator = ((middle.y - bottom.y) * (top.x - bottom.x)
		+ (bottom.x - middle.x) * (top.y - bottom.y));

	output.A =
		((middle.y - bottom.y) * (encodedVertex.x - bottom.x)
			+ (bottom.x - middle.x) * (encodedVertex.y - bottom.y))
		/ denominator;
	output.B = 
		((bottom.y - top.y) * (encodedVertex.x - bottom.x) 
			+ (top.x - bottom.x) * (encodedVertex.y - bottom.y))
		/ denominator;
	output.C = 1 - output.A - output.B;

	return output;
}
