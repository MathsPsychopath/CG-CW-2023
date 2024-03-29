#include "Camera.h"

Camera::Camera(float x, float y, float z) {
	this->cameraPosition = glm::vec3(x,y,z);
	this->viewMatrix = glm::mat3();
}

void Camera::translate(glm::vec3 movementVector) {
	this->cameraPosition += movementVector;
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
	this->lookAt({ 0,0,0 });
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

void Camera::useAnimation(float& progress, int stage, RenderType& renderer, std::set<std::string>& hiddenObjects, Lighting& lighting, bool& isCameraMoving, glm::vec3& lightPosition) {
	if (stage == 0) {
		// trucking movement to go past the cornell box
		renderer = RASTER;
		glm::vec3 start(-3, 0, 4);
		glm::vec3 initialDirection(-1, 0, 4);
		glm::vec3 finalDirection(1, 0, 4);
		glm::vec3 end(3, 0, 4);

		useBezierPosition(progress, start, initialDirection, finalDirection, end);
		progress += 1.0/90;
	}
	else if (stage == 1) {
		// turn towards the box
		lookAt({ 3 - 3*progress,0,0});
		progress += 0.025;
	}
	else if (stage == 2) {
		// look at box whilst retracing halfway
		lookAt({ 0,0,0 });
		glm::vec3 start(3, 0, 4);
		glm::vec3 initialDirection(2, 0, 4);
		glm::vec3 finalDirection(1, 0, 4);
		glm::vec3 end(0, 0, 4);
		useBezierPosition(progress, start, initialDirection, finalDirection, end);
		progress += 0.025;
	}
	else if (stage == 3) {
		// half orbit the box with pointcloud
		renderer = POINTCLOUD;
		rotate(-1, 0, 0);
		progress += 1.0 / 90;
		lookAt({ 0,0,0 });
	}
	else if (stage == 4) {
		// orbit the box
		renderer = WIREFRAME;
		rotate(-1, 0, 0);
		progress += 1.0/90;
		lookAt({ 0,0,0 });
	}
	else if (stage == 5) {
		renderer = RASTER;
		rotate(1, 1, 0);
		progress += 1.0 / 125;
		lookAt({ 0,0,0 });
	}
	else if (stage == 6) {
		// dolly into the box
		translate({ 0,0,-0.05 });
		progress += 0.025;
	}
	else if (stage == 7) {
		// go right top middle
		if (progress > 0.5) {
			renderer = RAYTRACE;
			lighting.useAmbience = true;
			lighting.useShadow = true;
		}
		hiddenObjects.insert("ceiling");
		hiddenObjects.insert("right_wall");
		glm::vec3 start(0.07, 0.065, 1.5);
		glm::vec3 initialDirection(1, 0.9, 0);
		glm::vec3 finalDirection(2, 0.9, 4);
		glm::vec3 end(0.9, 0.9, 0);
		useBezierPosition(progress, start, initialDirection, finalDirection, end);
		lookAt({ 0,0,0 });
		progress += 0.025;
	}
	else if (stage == 8) {
		if (progress > 0.2) {
			hiddenObjects.erase("red_sphere");
			hiddenObjects.erase("ceiling");
			lighting.useIncidence = true;
		}
		if (progress > 0.5) {
			lighting.useSpecular = true;
			lighting.useShadow = false;
		}
		if (progress > 0.95) {
			lighting.useReflections = true;
			hiddenObjects.erase("right_wall");
			hiddenObjects.erase("ceiling");
		}
		
		glm::vec3 start(0.9, 0.9, 0);
		glm::vec3 initialDirection(1, 1, 0);
		glm::vec3 finalDirection(1, 0, -1);
		glm::vec3 end(0, 0.9, 1);
		useBezierPosition(progress, start, initialDirection, finalDirection, end);
		lookAt({ 0,0,0 });
		progress += 0.005;
	}
	else if (stage == 9) {
		lighting.useSoftShadow = true;
		if (progress > 0.3) {
			lighting.useFilter = true;
		}
		if (progress > 0.7) {
			lighting.usePhong = true;
			lighting.useIncidence = false;
			lighting.useProximity = false;
		}
		glm::vec3 start(0, 0.9, 1);
		glm::vec3 initialDirection(-1, 0.9, 0);
		glm::vec3 finalDirection(1, 0.7, -1);
		glm::vec3 end(0, 0.9, -0.7);
		useBezierPosition(progress, start, initialDirection, finalDirection, end);
		lookAt({ 0,0,0 });
		progress += 0.005;
	}
	else if (stage == 10) {
		if (progress > 0.2) {
			lighting.useFilter = false;
			lighting.useProximity = true;
		}
		if (progress > 0.6) {
			lighting.useIncidence = true;
			lighting.usePhong = true;
		}
		glm::vec3 start(0, 0.9, -0.7);
		glm::vec3 initialDirection(0.5, 0.5, -0.2);
		glm::vec3 finalDirection(-1, 1, -1);
		glm::vec3 end(0, 0, 2);
		useBezierPosition(progress, start, initialDirection, finalDirection, end);
		lookAt({ 0,0,0 });
		progress += 0.01;
	}
	else if (stage == 11) {
		lighting.useSoftShadow = false;
		lighting.useShadow =true;
		glm::vec3 start(0, 0.5, 0.75);
		glm::vec3 initialDirection(-0.5, 0.75, 1);
		glm::vec3 finalDirection(0.5, -0.25, 0.5);
		glm::vec3 end(0, 0.75, 1);
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
		lightPosition = point;
		progress += 0.01;
	}
	else if (stage == 12) {
		glm::vec3 start(0,0.75, 1);
		glm::vec3 initialDirection(-0.5, 0, 0.75);
		glm::vec3 finalDirection(0.5, 0.25, 0.5);
		glm::vec3 end(-0.25, 0.75, 1);
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
		lightPosition = point;
		progress += 0.01;
	}
	else {
		isCameraMoving = false;
		std::cout << "trigger" << std::endl;
	}		
}