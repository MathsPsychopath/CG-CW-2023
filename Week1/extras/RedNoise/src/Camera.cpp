#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(float x, float y, float z) {
	this->cameraPosition = glm::vec3(x,y,z);
}

void Camera::translate(glm::vec3 movementVector) {
	this->cameraPosition += movementVector;
}

void Camera::rotate(float xAnticlockwiseDegree, float yAnticlockwiseDegree) {
	const float xRad = glm::radians(xAnticlockwiseDegree);
	const float yRad = glm::radians(yAnticlockwiseDegree);

	const glm::mat3 xRotationMatrix = { 
		1.0, 0.0, 0.0,
		0.0, std::cos(xRad), std::sin(xRad),
		0.0, -std::sin(xRad), std::cos(xRad),
	};
	
	const glm::mat3 yRotationMatrix = {
		std::cos(yRad), 0.0, -std::sin(yRad),
		0.0, 1.0, 0.0,
		std::sin(yRad), 0.0, std::cos(yRad),
	};

	this->cameraPosition = xRotationMatrix*this->cameraPosition;
	this->cameraPosition = yRotationMatrix*this->cameraPosition;
}

glm::mat4 Camera::lookAt(glm::vec3 target) {
	// calculates the forward, right, up vectors given the target and the world vertical
	glm::mat4 viewMatrix = glm::lookAt(this->cameraPosition, target, glm::vec3(0, 1, 0));
	return viewMatrix;
}