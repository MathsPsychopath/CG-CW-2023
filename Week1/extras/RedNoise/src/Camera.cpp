#include "Camera.h"

Camera::Camera(float x, float y, float z) {
	this->cameraPosition = glm::vec3(x,y,z);
}

void Camera::translate(glm::vec3 movementVector) {
	this->cameraPosition += movementVector;
}

void Camera::rotate(float xAnticlockwiseDegree, float yAnticlockwiseDegree) {
	const glm::mat3 xRotationMatrix = { 
		1.0, 0.0, 0.0,
		0.0, std::cos(xAnticlockwiseDegree), std::sin(xAnticlockwiseDegree),
		0.0, -std::sin(xAnticlockwiseDegree), std::cos(xAnticlockwiseDegree),
	};
	
	const glm::mat3 yRotationMatrix = {
		std::cos(yAnticlockwiseDegree), 0.0, -std::sin(yAnticlockwiseDegree),
		0.0, 1.0, 0.0,
		std::sin(yAnticlockwiseDegree), 0.0, std::cos(yAnticlockwiseDegree),
	};

	this->cameraPosition = xRotationMatrix*this->cameraPosition;
	this->cameraPosition = yRotationMatrix*this->cameraPosition;
}