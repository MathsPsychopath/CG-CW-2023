#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include "FileReader.h"
#include "Constants.h"
#include <glm/gtx/string_cast.hpp>
#include "Rasterize.h"
#include "Wireframe.h"
#include "Raytrace.h"

void drawInterpolationRenders(DrawingWindow& window, Camera &camera, std::vector<ModelTriangle> objects, RenderType type) {
	window.clearPixels();
	glm::mat3 viewMatrix = camera.lookAt({ 0,0,0 }); 
	std::vector<std::vector<float>> zDepth(HEIGHT, std::vector<float>(WIDTH, std::numeric_limits<float>::max()));
	for (const ModelTriangle& object : objects) {
		CanvasPoint first = Wireframe::canvasIntersection(camera, object.vertices[0], 2.0, viewMatrix);
		CanvasPoint second = Wireframe::canvasIntersection(camera, object.vertices[1], 2.0, viewMatrix);
		CanvasPoint third = Wireframe::canvasIntersection(camera, object.vertices[2], 2.0, viewMatrix);

		CanvasTriangle flattened(first, second, third);
		if (type == POINTCLOUD) Wireframe::drawCloudPoints(window, camera, { first, second, third });
		else if (type == WIREFRAME) Wireframe::drawStrokedTriangle(window, flattened, Colour(255, 255, 255));
		else if (type == RASTER) Rasterize::drawRasterizedTriangle(window, flattened, object.colour, zDepth);
		
	}
}

void draw(DrawingWindow& window, Camera& camera, std::vector<ModelTriangle> objects) {
	window.clearPixels();
	glm::mat3 inverseViewMatrix = glm::inverse(camera.lookAt({ 0,0,0 }));
	glm::vec3 lightPosition = { 0, 0.75, 1.5 };
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			// normalise the canvas coordinates into real world coordinates
			glm::vec3 canvasPosition = Raytrace::getCanvasPosition(camera, CanvasPoint(x, y), inverseViewMatrix);

			glm::vec3 direction = glm::normalize(camera.cameraPosition - canvasPosition);

			RayTriangleIntersection intersection = Raytrace::getClosestValidIntersection(camera.cameraPosition, direction, objects);
			if (intersection.triangleIndex == -1) {
				continue;
			}

			glm::vec3 offsetPoint = intersection.intersectionPoint + 0.01f * intersection.intersectedTriangle.normal;
			glm::vec3 lightDirection = glm::normalize(lightPosition - offsetPoint);
			
			RayTriangleIntersection shadowIntersection = Raytrace::getClosestValidIntersection(offsetPoint, lightDirection, objects, intersection.triangleIndex);
			Colour color = intersection.intersectedTriangle.colour;
			if (shadowIntersection.triangleIndex != -1) {
				uint32_t pixelColor = (255 << 24) + (int(color.red / 3) << 16) + (int(color.green / 3) << 8) + int(color.blue / 3);
				window.setPixelColour(x, y, pixelColor);
				continue;
			}
			uint32_t pixelColor = (255 << 24) + (int(color.red) << 16) + (int(color.green) << 8) + int(color.blue);
			window.setPixelColour(x, y, pixelColor);
		}
	}
}

void handleEvent(SDL_Event event, DrawingWindow &window, Camera &camera, RenderType& type) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) camera.rotate(0, -1);
		else if (event.key.keysym.sym == SDLK_RIGHT) camera.rotate(0, 1);
		else if (event.key.keysym.sym == SDLK_UP) camera.translate(glm::vec3(0, 0, -0.1));
		else if (event.key.keysym.sym == SDLK_DOWN) camera.translate(glm::vec3(0, 0, 0.1));
		else if (event.key.keysym.sym == SDLK_w) camera.translate(glm::vec3(0, 0.1, 0));
		else if (event.key.keysym.sym == SDLK_a) camera.translate(glm::vec3(-0.1, 0, 0));
		else if (event.key.keysym.sym == SDLK_s) camera.translate(glm::vec3(0, -0.1, 0));
		else if (event.key.keysym.sym == SDLK_d) camera.translate(glm::vec3(0.1, 0, 0));
		else if (event.key.keysym.sym == SDLK_f) type = WIREFRAME;
		else if (event.key.keysym.sym == SDLK_r) type = RASTER;
		else if (event.key.keysym.sym == SDLK_t) type = RAYTRACE;
		else if (event.key.keysym.sym == SDLK_p) type = POINTCLOUD;

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
	RenderType type = RAYTRACE;

	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window, camera, type);
		//Triangle::drawRasterizedTriangle(window, triangle, textures);
		//drawPointCloud(window, cameraPosition, fr.loadedVertices);
		if (type == RAYTRACE) draw(window, camera, objects);
		else drawInterpolationRenders(window, camera, objects, type);

		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
