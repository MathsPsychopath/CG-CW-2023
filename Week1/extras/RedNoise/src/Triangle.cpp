#include "Triangle.h"

// sorts vertices by y-value ascending
void Triangle::sortTriangle(std::array<CanvasPoint, 3>& vertices){
	if (vertices[0].y > vertices[1].y) {
		std::swap(vertices[0], vertices[1]);
	}
	if (vertices[1].y > vertices[2].y) {
		std::swap(vertices[1], vertices[2]);
	}
	if (vertices[0].y > vertices[1].y) {
		std::swap(vertices[0], vertices[1]);
	}
}

void Triangle::drawLine(DrawingWindow& window, CanvasPoint start, CanvasPoint end, Colour color) {
	// these are floats because of division
	float xDiff = end.x - start.x;
	float yDiff = end.y - start.y;
	int stepCount = std::max(std::abs(xDiff), std::abs(yDiff));
	float xStepSize = xDiff / stepCount;
	float yStepSize = yDiff / stepCount;
	uint32_t pixelColor = (255 << 24) + (int(color.red) << 16) + (int(color.green) << 8) + int(color.blue);
	for (int i = 0; i < stepCount; i++) {
		int x = std::round(start.x + xStepSize * i);
		int y = std::round(start.y + yStepSize * i);
		window.setPixelColour(x, y, pixelColor);
	}
}

void Triangle::drawStrokedTriangle(DrawingWindow& window, CanvasTriangle triangle, Colour color) {
	drawLine(window, triangle.v0(), triangle.v1(), color);
	drawLine(window, triangle.v1(), triangle.v2(), color);
	drawLine(window, triangle.v0(), triangle.v2(), color);
}

void Triangle::drawRasterizedTriangle(DrawingWindow& window, CanvasTriangle triangle, Colour color) {
	// translate vertices to interface type
	std::array<CanvasPoint, 3> vertices = { triangle.v0(), triangle.v1(), triangle.v2() };

	// sort triangle and interpolate
	sortTriangle(vertices);
	InterpolatedTriangle interpolations = Interpolate::triangle(vertices);

	// rasterize top triangle
	uint32_t pixelColor = (255 << 24) + (int(color.red) << 16) + (int(color.green) << 8) + int(color.blue);
	for (int y = vertices[0].y, i = 0; y < vertices[1].y; y++, i++) {
		for (int x = std::floor(interpolations.topLeft[i]); x < std::ceil(interpolations.topRight[i]); x++) {
			window.setPixelColour(x, y, pixelColor);
		}
	}
	// rasterize bottom triangle
	for (int y = vertices[1].y, i = 0; y < vertices[2].y; y++, i++) {
		for (int x = std::floor(interpolations.leftBottom[i]); x < std::ceil(interpolations.rightBottom[i]); x++) {
			window.setPixelColour(x, y, pixelColor);
		}
	}
	drawStrokedTriangle(window, triangle, Colour(255, 255, 255));
}

uint32_t Triangle::getTexture(BarycentricCoordinates coordinates, std::array<CanvasPoint, 3> sortedVertices, TextureMap textures){
	int width = textures.width;
	int height = textures.height;
	glm::vec2 textureA(sortedVertices[0].texturePoint.x, sortedVertices[0].texturePoint.y);
	glm::vec2 textureB(sortedVertices[1].texturePoint.x, sortedVertices[1].texturePoint.y);
	glm::vec2 textureC(sortedVertices[2].texturePoint.x, sortedVertices[2].texturePoint.y);
	glm::vec2 textureCoordinate =
		textureA * coordinates.A +
		textureB * coordinates.B +
		textureC * coordinates.C;

	return textures.pixels[std::floor(textureCoordinate.x) + std::floor(textureCoordinate.y) * width];
}

void Triangle::drawRasterizedTriangle(DrawingWindow& window, CanvasTriangle triangle, TextureMap textures) {
	// translate vertices to interface type
	std::array<CanvasPoint, 3> canvasVertices = { triangle.v0(), triangle.v1(), triangle.v2() };

	// sort canvas triangle and interpolate
	sortTriangle(canvasVertices);
	std::cout << canvasVertices[0] << canvasVertices[1] << canvasVertices[2] << std::endl;
	InterpolatedTriangle interpolations = Interpolate::triangle(canvasVertices);

	// rasterize top triangle with textures
	for (int y = canvasVertices[0].y, i = 0; y < canvasVertices[1].y; y++, i++) {
		for (int x = std::floor(interpolations.topLeft[i]); x < std::ceil(interpolations.topRight[i]); x++) {
			glm::vec2 currentVertex(x, y);
			BarycentricCoordinates ratios = barycentric(canvasVertices, currentVertex);
			uint32_t pixelTexture = getTexture(ratios, canvasVertices, textures);
			window.setPixelColour(x, y, pixelTexture);
		}
	}

	// rasterize bottom triangle with textures
	for (int y = canvasVertices[1].y, i = 0; y < canvasVertices[2].y; y++, i++) {
		for (int x = std::floor(interpolations.leftBottom[i]); x < std::ceil(interpolations.rightBottom[i]); x++) {
			glm::vec2 currentVertex(x, y);
			BarycentricCoordinates ratios = barycentric(canvasVertices, currentVertex);
			uint32_t pixelTexture = getTexture(ratios, canvasVertices, textures);
			window.setPixelColour(x, y, pixelTexture);
		}
	}
}

void Triangle::drawRandomTriangle(DrawingWindow& window, bool isFilled){
	Colour color = Colour(rand() % 256, rand() % 256, rand() % 256);
	CanvasPoint vertex1 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT);
	CanvasPoint vertex2 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT);
	CanvasPoint vertex3 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT);
	if (isFilled) {
		drawRasterizedTriangle(window, CanvasTriangle(vertex1, vertex2, vertex3), color);
	}
	else {
		drawStrokedTriangle(window, CanvasTriangle(vertex1, vertex2, vertex3), color);
	}
}

BarycentricCoordinates Triangle::barycentric(const std::array<CanvasPoint, 3>& sortedVertices, glm::vec2 encodedVertex) {
	CanvasPoint A = sortedVertices[0];
	CanvasPoint B = sortedVertices[1];
	CanvasPoint C = sortedVertices[2];
	BarycentricCoordinates output = {};

	// cache the denominator as it is the same value for both
	float denominator = ((B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y));

	// calculate the ratio of distances from each vertex
	// Area of PBC / Area of ABC
	output.A = ((B.y - C.y) * (encodedVertex.x - C.x) + (C.x - B.x) * (encodedVertex.y - C.y)) / denominator;

	// Area of APC / Area of ABC
	output.B = ((C.y - A.y) * (encodedVertex.x - C.x) + (A.x - C.x) * (encodedVertex.y - C.y)) / denominator;

	output.C = 1 - output.A - output.B;

	return output;
}
