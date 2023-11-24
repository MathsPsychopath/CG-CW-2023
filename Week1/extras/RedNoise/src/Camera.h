#pragma once
#include "Constants.h"
#include <vector>
#include <glm/glm.hpp>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <set>

class Camera {
private:
	void useBezierPosition(float progress, glm::vec3 start, glm::vec3 initialDirection, glm::vec3 finalDirection, glm::vec3 end);

public:
	glm::vec3 cameraPosition;
	glm::mat3 viewMatrix;

	Camera(float x, float y, float z);

	void translate(glm::vec3 movementVector);

	void rotate(float xAnticlockwiseDegree, float yAnticlockwiseDegree, float zAnticlockwiseDegree);

	void lookAt(glm::vec3 target);

	void useAnimation(float& progress, int stage, RenderType& renderer, std::set<std::string>& hiddenObjects);
};