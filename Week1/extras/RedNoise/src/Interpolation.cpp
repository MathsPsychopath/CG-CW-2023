#include "Interpolation.h"

namespace {
	std::array<CanvasPoint, 4> projectLeftVertex(const std::array<CanvasPoint, 3>& sortedVertices) {
		// Compute projection, which is the linear interpolation formula rearranged for x
		float xProjection = sortedVertices[0].x
			+ ((sortedVertices[1].y - sortedVertices[0].y) / (sortedVertices[2].y - sortedVertices[0].y))
			* (sortedVertices[2].x - sortedVertices[0].x);

		CanvasPoint projectedPoint = CanvasPoint(std::round(xProjection), sortedVertices[1].y);

		// decide the left most vertex
		CanvasPoint left = projectedPoint.x < sortedVertices[1].x ? projectedPoint : sortedVertices[1];
		CanvasPoint right = left.x == projectedPoint.x ? sortedVertices[1] : projectedPoint;

		std::array<CanvasPoint, 4> output = { sortedVertices[0], left, right, sortedVertices[2] };
		return output;
	}
}

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

InterpolatedTriangle Interpolate::triangle(const std::array<CanvasPoint, 3>& sortedVertices) {

	std::array<CanvasPoint, 4> vertices = projectLeftVertex(sortedVertices);

	InterpolatedTriangle output;

	// interpolate x values for edges
	output.topLeft = singleFloat(vertices[0].x, vertices[1].x, std::abs(vertices[1].y - vertices[0].y) + 1);
	output.topRight = singleFloat(vertices[0].x, vertices[2].x, std::abs(vertices[2].y - vertices[0].y) + 1);
	output.leftBottom = singleFloat(vertices[1].x, vertices[3].x, std::abs(vertices[3].y - vertices[1].y) + 1);
	output.rightBottom = singleFloat(vertices[2].x, vertices[3].x, std::abs(vertices[3].y - vertices[2].y) + 1);

	return output;
}

CanvasPoint Interpolate::canvasIntersection(Camera &camera, glm::vec3 vertexPosition, float focalLength, const glm::mat4& viewMatrix) {
	
	// apply any view matrices supplied
	glm::vec4 position = viewMatrix * glm::vec4(vertexPosition, 1);

	glm::vec3 deviceCoordinates = glm::vec3(position) / position.w;
	float heightFocalLength = focalLength / (float)HEIGHT;
	float widthFocalLength = focalLength / (float)WIDTH;
	
	float u = (deviceCoordinates.x * 0.5 * widthFocalLength + 0.5) * WIDTH;
	float v = (deviceCoordinates.y * 0.5 * heightFocalLength + 0.5) * HEIGHT;

	// fixes horizontal flip and vertical flip
	u = WIDTH - u;
	v = HEIGHT - v;
	return CanvasPoint(u,v, -deviceCoordinates.z);
}
