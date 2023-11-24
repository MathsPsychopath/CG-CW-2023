#include "Camera.h"

Camera::Camera(float x, float y, float z) {
	this->cameraPosition = glm::vec3(x,y,z);
	this->viewMatrix = glm::mat3();
}

void Camera::translate(glm::vec3 movementVector) {
	this->cameraPosition += movementVector;
}

void Camera::rotate(float xAnticlockwiseDegree, float yAnticlockwiseDegree, float zAnticlockwiseDegree) {
	const float xRad = glm::radians(xAnticlockwiseDegree);
	const float yRad = glm::radians(yAnticlockwiseDegree);
	const float zRad = glm::radians(zAnticlockwiseDegree);

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

	const glm::mat3 zRotationMatrix = {
		std::cos(zRad), std::sin(zRad), 0,
		-std::sin(zRad), std::cos(zRad), 0,
		0, 0, 1
	};

	this->cameraPosition = xRotationMatrix * this->cameraPosition;
	this->cameraPosition = yRotationMatrix * this->cameraPosition;
	this->cameraPosition = zRotationMatrix * this->cameraPosition;
}

void Camera::lookAt(glm::vec3 target) {
	// calculates the forward, right, up vectors given the target and the world vertical
	glm::vec3 forward = glm::normalize(this->cameraPosition - target);

	glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), forward));

	glm::vec3 up = glm::normalize(glm::cross(forward, right));

	// mat3 is column-major, so we supply vectors directly
	glm::mat3 viewMatrix = glm::mat3(right, up, forward);
	this->viewMatrix = viewMatrix;
}

void Camera::useBezierPosition(float progress, glm::vec3 start, glm::vec3 initialDirection, glm::vec3 finalDirection, glm::vec3 end) {
	// progress is between 0 and 1
	float inverseProgress = 1 - progress;
	float quadProgress = glm::pow(progress, 2);
	float quadInvProgress = glm::pow(inverseProgress, 2);
	float cubicInvProgress = glm::pow(inverseProgress, 3);
	float cubicProgress = glm::pow(progress, 3);

	// this is literally binomial expansion
	glm::vec3 point = cubicInvProgress * start;
	point += 3 * quadInvProgress * progress * initialDirection;
	point += 3 * inverseProgress * quadProgress * finalDirection;
	point += cubicProgress * end;

	this->cameraPosition = point;
}

void Camera::useAnimation(float& progress, int stage, RenderType& renderer, std::set<std::string>& hiddenObjects) {
	if (stage == 0) {
		// trucking movement to go past the cornell box
		renderer = RASTER;
		glm::vec3 start(-3, 0, 4);
		glm::vec3 initialDirection(-1, 0, 4);
		glm::vec3 finalDirection(1, 0, 4);
		glm::vec3 end(3, 0, 4);

		useBezierPosition(progress, start, initialDirection, finalDirection, end);
		progress += 0.00625;
	}
	else if (stage == 1) {
		// slowly turn towards the box
		lookAt({ 3 - 3*progress,0,0});
		progress += 0.00625;
	}
	else if (stage == 2) {
		// look at box whilst retracing halfway
		lookAt({ 0,0,0 });
		glm::vec3 start(3, 0, 4);
		glm::vec3 initialDirection(2, 0, 4);
		glm::vec3 finalDirection(1, 0, 4);
		glm::vec3 end(0, 0, 4);
		useBezierPosition(progress, start, initialDirection, finalDirection, end);
		progress += 0.0125;
	}
	else if (stage == 3) {
		// orbit the box in sinusoidal pattern
		renderer = POINTCLOUD;
		rotate(glm::sin(progress/ 9.5), 0.1375, 0);
		if (progress > 0.5) renderer = WIREFRAME;
		progress += float(1)/7200;
		lookAt({ 0,0,0 });
	}
	else if (stage == 4) {
		// dolly into the box
		renderer = WIREFRAME;
		translate({ 0,0,-0.01 });
		progress += 0.004;
	}
	else if (stage == 5) {
		// go right top middle
		renderer = RASTER;
		hiddenObjects.insert("ceiling");
		//std::cout << cameraPosition.x << ", " << cameraPosition.y << ", " << cameraPosition.z << std::endl;
		glm::vec3 start(0.07, 0.065, 1.5);
		glm::vec3 initialDirection(1, 0.9, 0);
		glm::vec3 finalDirection(2, 0.9, 4);
		glm::vec3 end(0.9, 0.9, 0);
		useBezierPosition(progress, start, initialDirection, finalDirection, end);
		lookAt({ 0,0,0 });
		progress += 0.0004;
	}
	else if (stage == 6) {
		glm::vec3 start(0.9, 0.9, 0);
		glm::vec3 initialDirection(1, 1, 0);
		glm::vec3 finalDirection(3, 0, -1);
		glm::vec3 end(0, 0.9, 1);
		useBezierPosition(progress, start, initialDirection, finalDirection, end);
		lookAt({ 0,0,0 });
		progress += 0.0004;
	}
	else if (stage == 7) {
		glm::vec3 start(0, 0.9, 1);
		glm::vec3 initialDirection(-1, 0.9, 0);
		glm::vec3 finalDirection(1, 0.7, -1);
		glm::vec3 end(0, 0.9, -0.7);
		useBezierPosition(progress, start, initialDirection, finalDirection, end);
		lookAt({ 0,0,0 });
		progress += 0.0004;
	}
	else {
		renderer = RAYTRACE;

	}
}
