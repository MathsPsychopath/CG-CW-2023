#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <Colour.h>
#include "FileReader.h"
#include "Triangle.h"
#include "Constants.h"
#include "Camera.h"

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

void drawPointCloud(DrawingWindow& window, Camera &camera, std::unordered_map<std::string, glm::vec3> loadedVertices) {
	for (const auto& pair : loadedVertices) {
		CanvasPoint point = Interpolate::canvasIntersection(camera, pair.second, 2.0);
		uint32_t color = (255 << 24) + (255 << 16) + (255 << 8) + 255;
		window.setPixelColour(point.x, point.y, color);
	}
}

void drawRaster(DrawingWindow& window, Camera &camera, std::vector<ModelTriangle> objects) {
	window.clearPixels();
	std::vector<std::vector<float>> zDepth(HEIGHT, std::vector<float>(WIDTH, std::numeric_limits<float>::max()));
	for (const ModelTriangle& object : objects) {
		CanvasPoint first = Interpolate::canvasIntersection(camera, object.vertices[0], 2.0);
		CanvasPoint second = Interpolate::canvasIntersection(camera, object.vertices[1], 2.0);
		CanvasPoint third = Interpolate::canvasIntersection(camera, object.vertices[2], 2.0);

		CanvasTriangle flattened(first, second, third);
		Triangle::drawRasterizedTriangle(window, flattened, object.colour, zDepth);
	}
}

void handleEvent(SDL_Event event, DrawingWindow &window, Camera &camera) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) camera.rotate(0, -0.1);
		else if (event.key.keysym.sym == SDLK_RIGHT) camera.rotate(0,0.1);
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl; // these last two sound like z axis
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
		else if (event.key.keysym.sym == SDLK_w) camera.translate(glm::vec3(0, 0.1, 0));
		else if (event.key.keysym.sym == SDLK_a) camera.translate(glm::vec3(-0.1,0,0));
		else if (event.key.keysym.sym == SDLK_s) camera.translate(glm::vec3(0,-0.1,0));
		else if (event.key.keysym.sym == SDLK_d) camera.translate(glm::vec3(0.1,0,0));
		/*else if (event.key.keysym.sym == SDLK_u) {
			Triangle::drawRandomTriangle(window, false);
			std::cout << "Drew Stroked Triangle" << std::endl;
		}*/
		/*else if (event.key.keysym.sym == SDLK_f) {
			Triangle::drawRandomTriangle(window, true);
			std::cout << "Drew Filled Triangle" << std::endl;
		}*/
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

	Camera camera(0.0, 0.0, 4.0);

	FileReader fr;
	fr.readMTLFile("cornell-box.mtl");
	std::vector<ModelTriangle> objects = fr.readOBJFile("cornell-box.obj", 0.35);

	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window, camera);
		//Triangle::drawRasterizedTriangle(window, triangle, textures);
		//drawPointCloud(window, cameraPosition, fr.loadedVertices);
		drawRaster(window, camera, objects);
		

		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
