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

void drawInterpolationRenders(DrawingWindow& window, Camera &camera, PolygonData& objects, RenderType type, TextureMap textures) {
	window.clearPixels();
	glm::mat3 viewMatrix = camera.lookAt({ 0,0,0 }); 
	std::vector<std::vector<float>> zDepth(HEIGHT, std::vector<float>(WIDTH, std::numeric_limits<float>::max()));
	for (int triangleIndex = 0; triangleIndex < objects.loadedTriangles.size(); triangleIndex++) {
		CanvasPoint first = Wireframe::canvasIntersection(camera, objects.getTriangleVertexPosition(triangleIndex, 0), 2.0, viewMatrix);
		CanvasPoint second = Wireframe::canvasIntersection(camera, objects.getTriangleVertexPosition(triangleIndex, 1), 2.0, viewMatrix);
		CanvasPoint third = Wireframe::canvasIntersection(camera, objects.getTriangleVertexPosition(triangleIndex, 2), 2.0, viewMatrix);

		CanvasTriangle flattened(first, second, third);
		if (type == POINTCLOUD) Wireframe::drawCloudPoints(window, camera, { first, second, third });
		else if (type == WIREFRAME) Wireframe::drawStrokedTriangle(window, flattened, Colour(255, 255, 255));
		else if (type == RASTER) {
			if (objects.loadedTriangles[triangleIndex].texturePoints[0] == -1) {
				Rasterize::drawRasterizedTriangle(window, flattened, objects.loadedTriangles[triangleIndex].colour, zDepth);
			}
			else {
				ModelTriangle texturedTriangle = objects.loadedTriangles[triangleIndex];
				first.texturePoint = objects.loadedTextures[texturedTriangle.texturePoints[0]];
				second.texturePoint = objects.loadedTextures[texturedTriangle.texturePoints[1]];
				third.texturePoint = objects.loadedTextures[texturedTriangle.texturePoints[2]];
				CanvasTriangle triangle = { first, second, third };
				Rasterize::drawRasterizedTriangle(window, triangle, textures, zDepth);
			}
		}
	}
}

BarycentricCoordinates getGouraudBarycentric(PolygonData& objects, int triangleIndex, glm::vec3 position) {
	GouraudVertex A = objects.getTriangleVertex(triangleIndex, 0);
	GouraudVertex B = objects.getTriangleVertex(triangleIndex, 1);
	GouraudVertex C = objects.getTriangleVertex(triangleIndex, 2);

	glm::vec3 AB = glm::vec3(B) - glm::vec3(A);
	glm::vec3 AC = glm::vec3(C) - glm::vec3(A);
	glm::vec3 PA = glm::vec3(A) - position;
	glm::vec3 PB = glm::vec3(B) - position;
	glm::vec3 PC = glm::vec3(C) - position;

	float areaABC = glm::length(glm::cross(AB, AC));
	float areaPBC = glm::length(glm::cross(PB, PC));
	float areaPCA = glm::length(glm::cross(PC, PA));

	BarycentricCoordinates result{};
	result.A = areaPBC / areaABC;
	result.B = areaPCA / areaABC;
	result.C = 1 - result.A - result.B;
	return result;
}

void draw(DrawingWindow& window, Camera& camera, PolygonData& objects, glm::vec3 lightPosition, bool useShadow) {
	window.clearPixels();
	glm::mat3 inverseViewMatrix = glm::inverse(camera.lookAt({ 0,0,0 }));
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			// normalise the canvas coordinates into real world coordinates
			glm::vec3 canvasPosition = Raytrace::getCanvasPosition(camera, CanvasPoint(x, y), inverseViewMatrix);

			glm::vec3 direction = glm::normalize(camera.cameraPosition - canvasPosition);

			// fire initial ray into scene
			RayTriangleIntersection intersection = Raytrace::getClosestValidIntersection(camera.cameraPosition, direction, objects);
			if (intersection.triangleIndex == -1) {
				continue;
			}
			
			// fire shadow ray into light
			glm::vec3 offsetPoint = intersection.intersectionPoint + 0.01f * intersection.intersectedTriangle.normal;
			glm::vec3 lightDirection = glm::normalize(lightPosition - offsetPoint);
			float lightDistance = glm::length(lightPosition - offsetPoint);
			RayTriangleIntersection shadowIntersection = Raytrace::getClosestValidIntersection(offsetPoint, lightDirection, objects, intersection.triangleIndex, lightDistance);
			if (useShadow && shadowIntersection.triangleIndex != -1) {
				continue;
			}

			auto& vertices = intersection.intersectedTriangle.vertices;
			BarycentricCoordinates barycentric = getGouraudBarycentric(objects, intersection.triangleIndex, intersection.intersectionPoint);
			
			Colour finalColor = intersection.intersectedTriangle.colour + 
				objects.loadedVertices[vertices[0]].renderedColor * barycentric.A +
				objects.loadedVertices[vertices[1]].renderedColor * barycentric.B + 
				objects.loadedVertices[vertices[2]].renderedColor * barycentric.C;
			window.setPixelColour(x, y, finalColor.asNumeric());
			window.renderFrame();
		}
	}
}

void handleEvent(SDL_Event event, DrawingWindow &window, Camera &camera, RenderType& renderer, LightOptions& lighting, glm::vec3& lightPosition, bool& hasParametersChanged) {
	if (event.type == SDL_KEYDOWN) {
		hasParametersChanged = true;
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
		else hasParametersChanged = false;
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

void preprocess(PolygonData& objects, glm::vec3 lightPosition, glm::vec3 cameraPosition, LightOptions& lighting, bool& hasParametersChanged) {
	hasParametersChanged = false;
	Colour globalAmbientColor(30, 20, 10);
	Colour globalLightColor(255, 255, 255);
	for (auto& vertex : objects.loadedVertices) {
		glm::vec3 lightDirection = glm::normalize(lightPosition - glm::vec3(vertex));
		float lightDistance = glm::distance(lightPosition, glm::vec3(vertex));

		// calculate the ambient lighting for vertex
		if (lighting.useAmbience) {
			vertex.ambient = globalAmbientColor;
		}

		// calculate the proximity lighting for vertex
		if (lighting.useProximity) {
			float lightIntensity = 5;
			float illumination = (lightIntensity / (4 * glm::pi<float>() * glm::pow(lightDistance, 2)));
			vertex.proximity = vertex.originalColor * illumination;
		}
		else vertex.proximity = Colour();

		// calculate the incidence lighting for vertex
		if (lighting.useIncidence) {
			float incidentAngle = glm::dot(vertex.normal, lightDirection);
			vertex.incidental = vertex.originalColor * glm::max(incidentAngle, 0.0f);
		}
		else vertex.incidental = Colour();

		// calculate the specular lighting for vertex
		if (lighting.useSpecular) {
			glm::vec3 reflection = glm::reflect(-lightDirection, vertex.normal);
			glm::vec3 viewDirection = glm::normalize(cameraPosition - glm::vec3(vertex));
			float specularity = glm::dot(reflection, viewDirection);
			float shininess = 256;
			vertex.specular = globalLightColor * glm::pow(glm::max(specularity, 0.0f), shininess);
		}
		else vertex.specular = Colour();

		vertex.renderedColor = (vertex.ambient + vertex.proximity + vertex.incidental + vertex.specular);
		std::cout << vertex << std::endl;

	}
	std::cout << "done" << std::endl;
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
	//Camera camera(0.0, 5.5, 4.0);

	FileReader fr;
	fr.readMTLFile("textured-cornell-box.mtl");
	PolygonData objects = fr.readOBJFile("textured-cornell-box.obj", 0.35, {textures.width, textures.height});
	//PolygonData objects = fr.readOBJFile("sphere.obj", 1);
	if (objects.loadedTriangles.empty()) return -1;
	
	// calculate normals
	for (int triangleIndex = 0; triangleIndex < objects.loadedTriangles.size(); triangleIndex++) {
		glm::vec3 e0 = objects.getTriangleVertexPosition(triangleIndex, 1)
			- objects.getTriangleVertexPosition(triangleIndex, 0);
		glm::vec3 e1 = objects.getTriangleVertexPosition(triangleIndex, 2) -
			objects.getTriangleVertexPosition(triangleIndex, 0);
		objects.loadedTriangles[triangleIndex].normal = glm::normalize(glm::cross(e0, e1)); // winding order for cornell box
		//objects.loadedTriangles[triangleIndex].normal = glm::normalize(glm::cross(e1, e0)); // winding order for sphere
	}
	
	// normalise all vertex normal sums.
	for (auto& entry : objects.vertexToTriangles) {
		glm::vec3 vertexNormal = { 0,0,0 };
		int vertexIndex = entry.first;
		std::set<int> triangleIndices = entry.second;
		for (int triangleIndex : triangleIndices) {
			vertexNormal += objects.loadedTriangles[triangleIndex].normal;
		}
		objects.loadedVertices[vertexIndex].normal = glm::normalize(vertexNormal);
	}

	RenderType renderer = RASTER;
	LightOptions lighting(true, true, true, true, true);
	glm::vec3 lightPosition = { 0, 0.5, 0.25 };
	//glm::vec3 lightPosition = { 1, 5, 1 };
	bool hasParametersChanged = true;
	preprocess(objects, lightPosition, camera.cameraPosition, lighting, hasParametersChanged);

	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window, camera, renderer, lighting, lightPosition, hasParametersChanged);
		if (renderer == RAYTRACE) {
			if (hasParametersChanged) preprocess(objects, lightPosition, camera.cameraPosition, lighting, hasParametersChanged);
			draw(window, camera, objects, lightPosition, lighting.useShadow);
		}
		else drawInterpolationRenders(window, camera, objects, renderer, textures);

		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
