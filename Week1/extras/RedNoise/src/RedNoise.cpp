#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include "FileReader.h"
#include "Constants.h"
#include <glm/gtx/string_cast.hpp>
#include "Rasterize.h"
#include "Wireframe.h"
#include "Raytrace.h"
#include "Lighting.h"

void drawInterpolationRenders(DrawingWindow& window, Camera &camera, std::vector<ModelTriangle> objects, RenderType type) {
	window.clearPixels();
	glm::mat3 viewMatrix = camera.lookAt({ 0,0,0 }); 
	std::vector<std::vector<float>> zDepth(HEIGHT, std::vector<float>(WIDTH, std::numeric_limits<float>::max()));
	for (const ModelTriangle& object : objects) {
		CanvasPoint first = Wireframe::canvasIntersection(camera, object.vertices[0].position, 2.0, viewMatrix);
		CanvasPoint second = Wireframe::canvasIntersection(camera, object.vertices[1].position, 2.0, viewMatrix);
		CanvasPoint third = Wireframe::canvasIntersection(camera, object.vertices[2].position, 2.0, viewMatrix);

		CanvasTriangle flattened(first, second, third);
		if (type == POINTCLOUD) Wireframe::drawCloudPoints(window, camera, { first, second, third });
		else if (type == WIREFRAME) Wireframe::drawStrokedTriangle(window, flattened, Colour(255, 255, 255));
		else if (type == RASTER) Rasterize::drawRasterizedTriangle(window, flattened, object.colour, zDepth);
		
	}
}

void draw(DrawingWindow& window, Camera& camera, std::vector<ModelTriangle> objects, LightOptions& lighting, glm::vec3 lightPosition) {
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
			float lightDistance = glm::length(lightPosition - offsetPoint);
			glm::vec3 lightDirection = glm::normalize(lightPosition - offsetPoint);
			Colour color = intersection.intersectedTriangle.colour;
			Colour ambience = Colour(color);

			RayTriangleIntersection shadowIntersection = Raytrace::getClosestValidIntersection(offsetPoint, lightDirection, objects, intersection.triangleIndex, lightDistance);
			
			if (lighting.useShadow && shadowIntersection.triangleIndex != -1) {
				if (lighting.useAmbience) color *= 0;
				else continue;
			}
			if (lighting.useProximity) {
				float lightIntensity = 5;
				float illumination = (lightIntensity / (4 * glm::pi<float>() * glm::pow(lightDistance,2)));
				color *= illumination;
			} 
			if (lighting.useIncidence) {
				float incidentAngle = glm::dot(intersection.intersectedTriangle.normal, lightDirection);
				incidentAngle = glm::max(incidentAngle, 0.f);
				color *= incidentAngle;
			}
			if (lighting.useSpecular) {
				// apply specular formula
				glm::vec3 reflection = glm::reflect(-lightDirection, intersection.intersectedTriangle.normal);
				glm::vec3 viewDirection = glm::normalize(camera.cameraPosition - intersection.intersectionPoint);
				float specularity = glm::dot(reflection, viewDirection);
				// the alteration to highlight is to add dissapation as lightDistance increases
				//specularity = glm::pow(glm::max(specularity, 0.0f, 64) * 128;
				specularity = glm::pow(specularity, 256) * 255;
				color += specularity;
			}
			if (lighting.useAmbience) {
				color.applyAmbience(0.4, ambience);
			}
			window.setPixelColour(x, y, color.asNumeric());
		}
	}
}

void handleEvent(SDL_Event event, DrawingWindow &window, Camera &camera, RenderType& renderer, LightOptions& lighting, glm::vec3& lightPosition) {
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
		else if (event.key.keysym.sym == SDLK_h) lighting.useShadow = !lighting.useShadow;
		else if (event.key.keysym.sym == SDLK_m) lighting.useAmbience = !lighting.useAmbience;
		else if (event.key.keysym.sym == SDLK_p) lighting.useProximity = !lighting.useProximity;
		else if (event.key.keysym.sym == SDLK_i) lighting.useIncidence = !lighting.useIncidence;
		else if (event.key.keysym.sym == SDLK_z) lighting.useSpecular = !lighting.useSpecular;
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

	//Camera camera(0.0, 0.0, 4.0);
	Camera camera(0.0, 5.5, 4.0);

	FileReader fr;
	fr.readMTLFile("cornell-box.mtl");
	//std::vector<ModelTriangle> objects = fr.readOBJFile("cornell-box.obj", 0.35);
	std::vector<ModelTriangle> objects = fr.readOBJFile("sphere.obj", 1);
	
	// calculate normals
	for (auto& triangle : objects) {
		glm::vec3 e0 = triangle.vertices[1].position - triangle.vertices[0].position;
		glm::vec3 e1 = triangle.vertices[2].position - triangle.vertices[0].position;
		//triangle.normal = glm::normalize(glm::cross(e0, e1)); // winding order for cornell box
		triangle.normal = glm::normalize(glm::cross(e1, e0)); // winding order for sphere
		
		for (auto& vertex : triangle.vertices) {
			vertex.normal += triangle.normal;
		}
	}
	
	// normalise all vertex normal sums
	for (auto& triangle : objects) {
		for (auto& vertex : triangle.vertices) {
			vertex.normal = glm::normalize(vertex.normal);
		}
	}

	RenderType renderer = RAYTRACE;
	LightOptions lighting(false, true, true, true, false);
	//glm::vec3 lightPosition = { 0, 0, 0.25 };
	glm::vec3 lightPosition = { 0, 2, 1 };

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
