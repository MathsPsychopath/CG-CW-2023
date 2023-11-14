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

void draw(DrawingWindow& window, Camera& camera, std::vector<ModelTriangle> objects, LightType lighting, glm::vec3 lightPosition) {
	window.clearPixels();
	glm::mat3 inverseViewMatrix = glm::inverse(camera.lookAt({ 0,0,0 }));
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
			
			if (lighting == HARD) Raytrace::drawHardShadows(window, CanvasPoint(x,y), intersection.intersectedTriangle.colour, shadowIntersection.triangleIndex == -1);
			else if (lighting == PROXIMITY) {
				float lightDistance = glm::length(lightPosition - offsetPoint);
				float illumination = (2 / (3 * glm::pi<float>() * glm::pow(lightDistance,2)));
				illumination = glm::clamp(illumination, 0.0f, 1.0f);
				Colour color = intersection.intersectedTriangle.colour;
				uint32_t pixelColor = (255 << 24) + 
					(int(color.red * illumination) << 16) + 
					(int(color.green * illumination) << 8) + 
					int(color.blue * illumination);
				window.setPixelColour(x, y, pixelColor);
			} 
			else if (lighting == INCIDENCE) {
				float incidentAngle = glm::dot(intersection.intersectedTriangle.normal, lightDirection);
				incidentAngle = glm::max(incidentAngle, 0.0f);
				Colour color = intersection.intersectedTriangle.colour;
				uint32_t pixelColor = (255 << 24) +
					(int(color.red * incidentAngle) << 16) +
					(int(color.green * incidentAngle) << 8) +
					int(color.blue * incidentAngle);
				window.setPixelColour(x, y, pixelColor);
			}
				
		}
	}
}

void handleEvent(SDL_Event event, DrawingWindow &window, Camera &camera, RenderType& renderer, LightType& lighting, glm::vec3& lightPosition) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) {
			if (renderer == RAYTRACE) lightPosition += glm::vec3(-0.25, 0, 0);
			else camera.rotate(0, -1);
		}
		else if (event.key.keysym.sym == SDLK_RIGHT) {
			if (renderer == RAYTRACE) lightPosition += glm::vec3(0.25, 0, 0);
			else camera.rotate(0, 1);
		}
		else if (event.key.keysym.sym == SDLK_UP) {
			if (renderer == RAYTRACE) lightPosition += glm::vec3(0, 0.25, 0);
			else camera.translate(glm::vec3(0, 0, -0.1));
		}
		else if (event.key.keysym.sym == SDLK_DOWN) {
			if (renderer == RAYTRACE) lightPosition += glm::vec3(0, -0.25, 0);
			else camera.translate(glm::vec3(0, 0, 0.1));
		}
		else if (event.key.keysym.sym == SDLK_w) {
			if (renderer == RAYTRACE) lightPosition += glm::vec3(0, 0, -0.25);
			else camera.translate(glm::vec3(0, 0.1, 0));
		}
		else if (event.key.keysym.sym == SDLK_s) {
			if (renderer == RAYTRACE) lightPosition += glm::vec3(0, 0, 0.25);
			else camera.translate(glm::vec3(0, -0.1, 0));
		}
		else if (event.key.keysym.sym == SDLK_a) camera.translate(glm::vec3(-0.1, 0, 0));
		else if (event.key.keysym.sym == SDLK_d) camera.translate(glm::vec3(0.1, 0, 0));
		else if (event.key.keysym.sym == SDLK_1) renderer = WIREFRAME;
		else if (event.key.keysym.sym == SDLK_2) renderer = RASTER;
		else if (event.key.keysym.sym == SDLK_3) renderer = POINTCLOUD;
		else if (event.key.keysym.sym == SDLK_4) renderer = RAYTRACE;
		else if (event.key.keysym.sym == SDLK_h) lighting = HARD;
		else if (event.key.keysym.sym == SDLK_p) lighting = PROXIMITY;
		else if (event.key.keysym.sym == SDLK_i) lighting = INCIDENCE;
		else if (event.key.keysym.sym == SDLK_z) lighting = SPECULAR;
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
	// calculate normals
	for (auto& triangle : objects) {
		glm::vec3 e0 = triangle.vertices[1] - triangle.vertices[0];
		glm::vec3 e1 = triangle.vertices[2] - triangle.vertices[0];
		triangle.normal = glm::normalize(glm::cross(e0, e1));
	}
	RenderType renderer = RAYTRACE;
	LightType lighting = INCIDENCE;
	glm::vec3 lightPosition = { 0, 0.75, 0 };

	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window, camera, renderer, lighting, lightPosition);
		//Triangle::drawRasterizedTriangle(window, triangle, textures);
		//drawPointCloud(window, cameraPosition, fr.loadedVertices);
		if (renderer == RAYTRACE) draw(window, camera, objects, lighting, lightPosition);
		else drawInterpolationRenders(window, camera, objects, renderer);

		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
