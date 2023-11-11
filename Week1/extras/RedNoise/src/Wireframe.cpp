#include "Wireframe.h"

void Wireframe::drawLine(DrawingWindow& window, CanvasPoint start, CanvasPoint end, Colour color) {
	// these are floats because of division
	float xDiff = end.x - start.x;
	float yDiff = end.y - start.y;
	int stepCount = std::max(std::abs(xDiff), std::abs(yDiff));
	float xStepSize = xDiff / stepCount;
	float yStepSize = yDiff / stepCount;
	uint32_t pixelColor = (255 << 24) + (int(color.red) << 16) + (int(color.green) << 8) + int(color.blue);
	for (int i = 0; i < stepCount; i++) {
		int x = std::round(start.x + xStepSize * i);
		if (x >= WIDTH || x < 0) continue;
		int y = std::round(start.y + yStepSize * i);
		if (y >= HEIGHT || y < 0) continue;
		window.setPixelColour(x, y, pixelColor);
	}
}

void Wireframe::drawStrokedTriangle(DrawingWindow& window, CanvasTriangle triangle, Colour color) {
	drawLine(window, triangle.v0(), triangle.v1(), color);
	drawLine(window, triangle.v1(), triangle.v2(), color);
	drawLine(window, triangle.v0(), triangle.v2(), color);
}

void Wireframe::drawCloudPoints(DrawingWindow& window, Camera& camera, std::vector<CanvasPoint> loadedVertices) {
	for (const auto& vertex: loadedVertices) {
		if (vertex.x < 0 || vertex.x >= WIDTH || vertex.y < 0 || vertex.y >= HEIGHT) continue;
		uint32_t color = (255 << 24) + (255 << 16) + (255 << 8) + 255;
		window.setPixelColour(vertex.x, vertex.y, color);
	}
}

CanvasPoint Wireframe::canvasIntersection(Camera& camera, glm::vec3 vertexPosition, float focalLength, const glm::mat3& viewMatrix) {

	// find the displacement relative to the camera, 
	// then get the position vector in terms of the camera's POV
	const glm::vec3 displacement = camera.cameraPosition - vertexPosition;
	const glm::vec3 adjustedVector = displacement * viewMatrix;

	int scaleFactor = 180;

	float u = focalLength * (adjustedVector.x / adjustedVector.z) * scaleFactor + float(WIDTH) / 2;

	float v = focalLength * (adjustedVector.y / adjustedVector.z) * scaleFactor + float(HEIGHT) / 2;
	// fixes horizontal flip
	u = WIDTH - u;
	return CanvasPoint(u, v, -adjustedVector.z);
}

void Wireframe::drawWireframe(DrawingWindow& window, Camera& camera, std::vector<ModelTriangle> objects) {
	for (const ModelTriangle& object : objects) {
		CanvasPoint first = canvasIntersection(camera, object.vertices[0], 2.0, glm::mat3(1.0));
		CanvasPoint second = canvasIntersection(camera, object.vertices[1], 2.0, glm::mat3(1.0));
		CanvasPoint third = canvasIntersection(camera, object.vertices[2], 2.0, glm::mat3(1.0));

		CanvasTriangle flattened(first, second, third);
		drawStrokedTriangle(window, flattened, Colour(255, 255, 255));
	}
}
