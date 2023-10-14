#include "Interpolation.h"

std::vector<float> Interpolate::singleFloat(float from, float to, int numberOfValues) {
	const float increment = (to - from) / (numberOfValues - 1);
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

	// Compute projection, which is the linear interpolation formula rearranged for x
	float xProjection = sortedVertices[0].x
		+ ((sortedVertices[1].y - sortedVertices[0].y) / (sortedVertices[2].y - sortedVertices[0].y))
		* (sortedVertices[2].x - sortedVertices[0].x);

	glm::vec2 projectedPoint = glm::vec2(std::round(xProjection), sortedVertices[1].y);

	// decide the left most vertex
	glm::vec2 left = projectedPoint.x < sortedVertices[1].x ? projectedPoint : sortedVertices[1];
	glm::vec2 right = left.x == projectedPoint.x ? sortedVertices[1] : projectedPoint;

	InterpolatedTriangle output;

	// interpolate x values for edges
	output.topLeftXValues = singleFloat(sortedVertices[0].x, left.x, std::abs(left.y - sortedVertices[0].y));
	output.topRightXValues = singleFloat(sortedVertices[0].x, right.x, std::abs(right.y - sortedVertices[0].y));
	output.leftBottomXValues = singleFloat(left.x, sortedVertices[2].x, std::abs(sortedVertices[2].y - left.y));
	output.rightBottomXValues = singleFloat(right.x, sortedVertices[2].x, std::abs(right.y - sortedVertices[2].y));

	return output;
}
