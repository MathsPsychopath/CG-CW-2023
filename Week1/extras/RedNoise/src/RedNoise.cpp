#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <Colour.h>
#include "FileReader.h"
#include "Triangle.h"
#include "Constants.h"
#include "Camera.h"
#include <glm/gtx/string_cast.hpp>

void drawPointCloud(DrawingWindow& window, Camera &camera, std::unordered_map<std::string, glm::vec3> loadedVertices) {
	for (const auto& pair : loadedVertices) {
		CanvasPoint point = Interpolate::canvasIntersection(camera, pair.second, 2.0);
		uint32_t color = (255 << 24) + (255 << 16) + (255 << 8) + 255;
		window.setPixelColour(point.x, point.y, color);
	}
}

void draw(DrawingWindow& window, Camera &camera, std::vector<ModelTriangle> objects) {
	window.clearPixels();
	camera.rotate(0, 0.1);
	glm::mat4 viewMatrix = camera.lookAt({ 0,0,0 }); 
	std::vector<std::vector<float>> zDepth(HEIGHT, std::vector<float>(WIDTH, std::numeric_limits<float>::max()));
	for (const ModelTriangle& object : objects) {
		CanvasPoint first = Interpolate::canvasIntersection(camera, object.vertices[0], 180, viewMatrix);
		CanvasPoint second = Interpolate::canvasIntersection(camera, object.vertices[1], 180, viewMatrix);
		CanvasPoint third = Interpolate::canvasIntersection(camera, object.vertices[2], 180, viewMatrix);

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
		draw(window, camera, objects);

		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
