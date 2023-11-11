#pragma once
#include "Constants.h"
#include <vector>
#include <glm/glm.hpp>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
	glm::vec3 cameraPosition;

	Camera(float x, float y, float z);

	void translate(glm::vec3 movementVector);

	void rotate(float xAnticlockwiseDegree, float yAnticlockwiseDegree);

	glm::mat3 lookAt(glm::vec3 target);
};