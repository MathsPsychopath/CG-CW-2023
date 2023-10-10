#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <CanvasPoint.h>
#include <Colour.h>
#include <TextureMap.h>

#define WIDTH 320
#define HEIGHT 240

std::vector<float> interpolateSingleFloats(float from, float to, int numberOfValues) {
	const float increment = (to - from) / (numberOfValues - 1);
	std::vector<float> output = {};
	for (int i = 0; i < numberOfValues; i++) {
		output.push_back(from + i * increment);
	}
	return output;
}

std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValues) {
	const int indices = numberOfValues - 1;
	glm::vec3 steps((to.x - from.x) / indices, (to.y - from.y) / indices, (to.z - from.z) / indices);
	std::vector<glm::vec3> output = {};
	for (int i = 0; i < numberOfValues; i++) {
		output.push_back(from + (steps * float(i)));
	}
	return output;
}

void drawLine(DrawingWindow& window, CanvasPoint start, CanvasPoint end, Colour color) {
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

void drawStrokedTriangle(DrawingWindow& window, CanvasTriangle triangle, Colour color) {
	drawLine(window, triangle.v0(), triangle.v1(), color);
	drawLine(window, triangle.v1(), triangle.v2(), color);
	drawLine(window, triangle.v0(), triangle.v2(), color);
}

void drawRasterizedTriangle(DrawingWindow& window, CanvasTriangle triangle, Colour color) {
	// order the list
	CanvasPoint vertices[3] = {triangle.v0(), triangle.v1(), triangle.v2()};
	if (vertices[0].y > vertices[1].y) {
		std::swap(vertices[0], vertices[1]);
	}
	if (vertices[1].y > vertices[2].y) {
		std::swap(vertices[1], vertices[2]);
	}
	if (vertices[0].y > vertices[1].y) {
		std::swap(vertices[0], vertices[1]);
	}

	// Compute projection, which is the linear interpolation formula rearranged for x
	float xProjection = vertices[0].x
		+ ((vertices[1].y - vertices[0].y) / (vertices[2].y - vertices[0].y)) 
		* (vertices[2].x - vertices[0].x);

	CanvasPoint projectedPoint = CanvasPoint(std::round(xProjection), vertices[1].y);

	// decide the left most vertex
	CanvasPoint left = projectedPoint.x < vertices[1].x ? projectedPoint : vertices[1];
	CanvasPoint right = left.x == projectedPoint.x ? vertices[1] : projectedPoint;

	// interpolate x values for edges
	std::vector<float> topLeftXValues = interpolateSingleFloats(vertices[0].x, left.x, std::abs(left.y - vertices[0].y));
	std::vector<float> topRightXValues = interpolateSingleFloats(vertices[0].x, right.x, std::abs(right.y - vertices[0].y));
	std::vector<float> leftBottomXValues = interpolateSingleFloats(left.x, vertices[2].x, std::abs(vertices[2].y - left.y));
	std::vector<float> rightBottomXValues = interpolateSingleFloats(right.x, vertices[2].x, std::abs(right.y - vertices[2].y));
	
	// rasterize top triangle
	uint32_t pixelColor = (255 << 24) + (int(color.red) << 16) + (int(color.green) << 8) + int(color.blue);
	for (int y = vertices[0].y, i = 0; y < right.y; y++, i++) {
		for (int x = std::floor(topLeftXValues[i]); x < std::ceil(topRightXValues[i]); x++) {
			window.setPixelColour(x, y, pixelColor);
		}
	}
	// rasterize bottom triangle
	for (int y = left.y, i = 0; y < vertices[2].y; y++, i++) {
		for (int x = std::floor(leftBottomXValues[i]); x < std::ceil(rightBottomXValues[i]); x++) {
			window.setPixelColour(x, y, pixelColor);
		}
	}
	drawStrokedTriangle(window, triangle, Colour(255, 255, 255));
}

void drawRasterizedTriangle(DrawingWindow& window, CanvasTriangle triangle, TextureMap textures) {}

void draw(DrawingWindow &window) {
	window.clearPixels();
	glm::vec3 topLeft(255, 0, 0);
	glm::vec3 topRight(0, 0, 255);
	glm::vec3 bottomRight(0, 255, 0);
	glm::vec3 bottomLeft(255, 255, 0);

	std::vector<glm::vec3> startColumnInterpolation = 
		interpolateThreeElementValues(topLeft, bottomLeft, window.height);
	std::vector<glm::vec3> endColumnInterpolation =
		interpolateThreeElementValues(topRight, bottomRight, window.height);

	for (size_t y = 0; y < window.height; y++) {
		std::vector<glm::vec3> rowInterpolation = 
			interpolateThreeElementValues(startColumnInterpolation[y], endColumnInterpolation[y], window.width);
		for (size_t x = 0; x < window.width; x++) {
			glm::vec3 currentPixel = rowInterpolation[x];
			float red = currentPixel.x;
			float green = currentPixel.y;
			float blue = currentPixel.z;
			uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue);
			window.setPixelColour(x, y, colour);
		}
	}
}

void drawRandomTriangle(DrawingWindow &window, bool isFilled) {
	Colour color = Colour(rand() % 256, rand() % 256, rand() % 256);
	CanvasPoint vertex1 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT);
	CanvasPoint vertex2 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT);
	CanvasPoint vertex3 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT);
	if (isFilled) {
		drawRasterizedTriangle(window, CanvasTriangle(vertex1, vertex2, vertex3), color);
	} else {
		drawStrokedTriangle(window, CanvasTriangle(vertex1, vertex2, vertex3), color);
	}
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
		else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
		else if (event.key.keysym.sym == SDLK_u) {
			drawRandomTriangle(window, false);
			std::cout << "Drew Stroked Triangle" << std::endl;
		}
		else if (event.key.keysym.sym == SDLK_f) {
			drawRandomTriangle(window, true);
			std::cout << "Drew Filled Triangle" << std::endl;
		}
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;

	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		//drawRasterizedTriangle(window, CanvasTriangle(CanvasPoint(WIDTH / 3, HEIGHT / 2), CanvasPoint((WIDTH * 2) / 3, HEIGHT / 3), CanvasPoint(WIDTH /2, 300)), Colour(40, 200, 40));
		//draw(window);
		/*drawLine(window, CanvasPoint(0, 0), CanvasPoint(WIDTH / 2, HEIGHT / 2), Colour(255, 255, 255));
		drawLine(window, CanvasPoint(WIDTH - 1, 0), CanvasPoint(WIDTH / 2, HEIGHT / 2), Colour(255, 255, 255));
		drawLine(window, CanvasPoint(WIDTH /2, 0), CanvasPoint(WIDTH / 2, HEIGHT - 1), Colour(255, 255, 255));
		drawLine(window, CanvasPoint(WIDTH / 3, HEIGHT / 2), CanvasPoint((WIDTH * 2) / 3, HEIGHT / 2), Colour(255, 255, 255));*/

		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
