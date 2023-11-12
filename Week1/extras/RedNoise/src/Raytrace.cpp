#include "Raytrace.h"

RayTriangleIntersection Raytrace::getClosestValidIntersection(glm::vec3 cameraPosition, glm::vec3 rayDirection, const std::vector<ModelTriangle>& objects) {
	RayTriangleIntersection closest;
	closest.distanceFromCamera = std::numeric_limits<float>::max();
	int index = 0;
	closest.triangleIndex = -1;
	for (const auto& triangle : objects) {
		glm::vec3 e0 = triangle.vertices[1] - triangle.vertices[0];
		glm::vec3 e1 = triangle.vertices[2] - triangle.vertices[0];
		glm::vec3 SPVector = cameraPosition - triangle.vertices[0];
		glm::mat3 DEMatrix(-rayDirection, e0, e1);
		glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
		float t = possibleSolution.x; // distance from camera
		float u = possibleSolution.y; // distance along v1-v0 edge
		float v = possibleSolution.z; // distance along v2-v0 edge
		
		// assert validity check
		if (u >= 0.0 && u <= 1.0 && v >= 0.0 && v <= 1.0 && u + v <= 1.0) {
			// get the closest triangle to camera
			if (t > closest.distanceFromCamera || t < 0) continue;
			closest.distanceFromCamera = t;
			closest.triangleIndex = index++;
			closest.intersectedTriangle = triangle;
			closest.intersectionPoint = cameraPosition + t * rayDirection;
		}
	}
	return closest;
}

glm::vec3 Raytrace::getCanvasPosition(Camera& camera, CanvasPoint position, glm::mat3 inverseViewMatrix) {
	int scaleFactor = 180;
	float focalLength = 2.0f;
	//position.x = WIDTH - position.x;
	float realX = ((position.x + float(WIDTH) / 2) / (scaleFactor * focalLength));
	float realY = ((position.y + float(HEIGHT) / 2) / (scaleFactor * focalLength));
	glm::vec3 displacement = inverseViewMatrix * glm::vec3(realX, realY, -1);
	return camera.cameraPosition + displacement;
}