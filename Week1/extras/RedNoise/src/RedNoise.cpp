#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>
#include <CanvasPoint.h>
#include <Colour.h>
#include <TextureMap.h>
#include "Interpolation.h"

#define WIDTH 320
#define HEIGHT 240

// left -> right, top -> bottom
void sortTriangle(std::array<CanvasPoint, 3>& vertices) {
	// Uses glm::vec2 because it has a common 2 member interface
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
	// translate vertices to interface type
	std::array<CanvasPoint, 3> vertices = { triangle.v0(), triangle.v1(), triangle.v2()};

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

uint32_t getTexture(BarycentricCoordinates coordinates, std::array<CanvasPoint, 3> sortedVertices, TextureMap textures) {
	int width = textures.width;
	int height = textures.height;
	glm::vec2 textureA(sortedVertices[0]/*.texturePoint*/.x, sortedVertices[0]/*.texturePoint*/.y);
	glm::vec2 textureB(sortedVertices[1]/*.texturePoint*/.x, sortedVertices[1]/*.texturePoint*/.y);
	glm::vec2 textureC(sortedVertices[2]/*.texturePoint*/.x, sortedVertices[2]/*.texturePoint*/.y);
	glm::vec2 textureCoordinate =
		textureA * coordinates.A +
		textureB * coordinates.B +
		textureC * coordinates.C; 

	return textures.pixels[std::floor(textureCoordinate.x + textureCoordinate.y * width)];
}

void drawRasterizedTriangle(DrawingWindow& window, CanvasTriangle triangle, TextureMap textures) {
	// translate vertices to interface type
	std::array<CanvasPoint, 3> canvasVertices = { triangle.v0(), triangle.v1(), triangle.v2()};

	// sort canvas triangle and interpolate
	sortTriangle(canvasVertices);
	std::cout << canvasVertices[0] << canvasVertices[1] << canvasVertices[2] << std::endl;
	InterpolatedTriangle interpolations = Interpolate::triangle(canvasVertices);

	// rasterize top triangle with textures
	for (int y = canvasVertices[0].y, i = 0; y < canvasVertices[1].y; y++, i++) {
		for (int x = std::floor(interpolations.topLeft[i]); x < std::ceil(interpolations.topRight[i]); x++) {
			glm::vec2 currentVertex(x, y);
			BarycentricCoordinates ratios = Interpolate::barycentric(canvasVertices, currentVertex);
			uint32_t pixelTexture = getTexture(ratios, canvasVertices, textures);
			window.setPixelColour(x, y, pixelTexture);
		}
	}

	// rasterize bottom triangle with textures
	for (int y = canvasVertices[1].y, i = 0; y < canvasVertices[2].y; y++, i++) {
		for (int x = std::floor(interpolations.leftBottom[i]); x < std::ceil(interpolations.rightBottom[i]); x++) {
			glm::vec2 currentVertex(x, y);
			BarycentricCoordinates ratios = Interpolate::barycentric(canvasVertices, currentVertex);
			uint32_t pixelTexture = getTexture(ratios, canvasVertices, textures);
			window.setPixelColour(x, y, pixelTexture);
		}
	}
}

void draw(DrawingWindow &window) {
	window.clearPixels();
	glm::vec3 topLeft(255, 0, 0);
	glm::vec3 topRight(0, 0, 255);
	glm::vec3 bottomRight(0, 255, 0);
	glm::vec3 bottomLeft(255, 255, 0);

	std::vector<glm::vec3> startColumnInterpolation = 
		Interpolate::threeElementValues(topLeft, bottomLeft, window.height);
	std::vector<glm::vec3> endColumnInterpolation =
		Interpolate::threeElementValues(topRight, bottomRight, window.height);

	for (size_t y = 0; y < window.height; y++) {
		std::vector<glm::vec3> rowInterpolation = 
			Interpolate::threeElementValues(startColumnInterpolation[y], endColumnInterpolation[y], window.width);
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

	CanvasTriangle triangle(CanvasPoint(160, 10), CanvasPoint(300, 230), CanvasPoint(10, 150));
	triangle.v0().texturePoint = TexturePoint(195, 5);
	triangle.v1().texturePoint = TexturePoint(395, 380);
	triangle.v2().texturePoint = TexturePoint(65, 330);
	TextureMap textures = TextureMap("texture.ppm");

	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		drawRasterizedTriangle(window, triangle, textures);
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
