#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

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

glm::mat3 Camera::lookAt(glm::vec3 target) {
	// forward vector
	const glm::vec3 forward = glm::normalize(this->cameraPosition - target);

	// get right by cross product of vertical (0,1,0) and forward
	const glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), forward));

	// get up by cross product of right and forward 
	const glm::vec3 up = glm::normalize(glm::cross(forward, right));
	/*std::cout << "up (invariant) " <<up.x << ", " << up.y << ", " << up.z << std::endl;
	std::cout << "right (invariant) " <<right.x << ", " << right.y << ", " << right.z << std::endl;
	std::cout << "forward (variant) " << forward.x << ", " << forward.y << ", " << forward.z << std::endl;*/

	glm::mat3 viewMatrix = glm::mat3(right, up, -forward);
	return viewMatrix;
}