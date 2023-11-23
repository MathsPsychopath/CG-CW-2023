#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include "FileReader.h"
#include <glm/gtx/string_cast.hpp>
#include "Rasterize.h"
#include "Wireframe.h"
#include "Raytrace.h"

void drawInterpolationRenders(DrawingWindow& window, Camera &camera, PolygonData& objects, RenderType type, TextureMap& textures) {
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

std::vector<std::vector<uint32_t>> getRaytrace(Camera& camera, PolygonData& objects, TextureMap& textures, glm::vec3 lightPosition) {
	// create results canvas
	glm::mat3 inverseViewMatrix = glm::inverse(camera.lookAt({ 0,0,0 }));
	std::vector<std::vector<uint32_t>> colorBuffer(HEIGHT, std::vector<uint32_t>(WIDTH));
	std::vector<std::thread> threads;
	// parallelise workload
	
	for (int i = 0; i < 4; i++) {
		int startY = (HEIGHT << 2) * i;
		int endY = (HEIGHT << 2) * (i + 1);
		threads.emplace_back(Raytrace::renderSegment, glm::vec2{startY, endY}, std::ref(colorBuffer), std::ref(objects), std::ref(camera), std::ref(textures), lightPosition);
	}
	for (auto& thread : threads) thread.join();
	return colorBuffer;
}

void useBilteralFilter(glm::vec2 boundY, std::vector<std::vector<uint32_t>>& colorBuffer) {
	// apply bilateral filter algorithm
}

void applyFilter(std::vector<std::vector<uint32_t>>& colorBuffer) {
	// parallelise workload here
	std::vector<std::thread> threads;
	for (int i = 0; i < 4; i++) {
		int startY = (HEIGHT << 2) * i;
		int endY = (HEIGHT << 2) * (i + 1);
		threads.emplace_back(useBilteralFilter, glm::vec2{ startY, endY }, std::ref(colorBuffer));
	}
	for (auto& thread : threads) thread.join();
}

void renderBuffer(std::vector<std::vector<uint32_t>>& colorBuffer, DrawingWindow& window) {
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			window.setPixelColour(x, y, colorBuffer[y][x]);
			window.renderFrame();
		}
	}
}

void handleEvent(SDL_Event event, DrawingWindow &window, Camera &camera, RenderType& renderer, glm::vec3& lightPosition, bool& hasParametersChanged) {
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
		int x, y;
		SDL_GetMouseState(&x, &y);
		std::cout << "{ x: " << x << ", y: " << y << " }" << std::endl;
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
	fr.readMTLFile("textured-cornell-box.mtl");
	PolygonData objects = fr.readOBJFile("textured-cornell-box.obj", 0.35, { textures.width, textures.height });
	//fr.appendPolygonData(objects, "sphere.obj");
	if (objects.loadedTriangles.empty()) return -1;
	
	glm::vec3 sceneMin(std::numeric_limits<float>::min());
	glm::vec3 sceneMax(std::numeric_limits<float>::max());
	for (int triangleIndex = 0; triangleIndex < objects.loadedTriangles.size(); triangleIndex++) {
		glm::vec3 v0 = objects.getTriangleVertexPosition(triangleIndex, 0);
		glm::vec3 v1 = objects.getTriangleVertexPosition(triangleIndex, 1);
		glm::vec3 v2 = objects.getTriangleVertexPosition(triangleIndex, 2);
		// calculate normals
		glm::vec3 e0 = v1 - v0;
		glm::vec3 e1 = v2 - v0;
		objects.loadedTriangles[triangleIndex].normal = glm::normalize(glm::cross(e0, e1));

		// calculate the bounding box for raytrace optimisation
		glm::vec3 minBound = glm::min(v0, v1, v2);
		glm::vec3 maxBound = glm::max(v0, v1, v2);
		objects.loadedTriangles[triangleIndex].boundingMinMax = {minBound, maxBound};
		sceneMax = glm::max(sceneMax, maxBound);
		sceneMin = glm::min(sceneMin, minBound);
	}
	objects.sceneBoundingMinMax = { sceneMin, sceneMax };
	
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

	RenderType renderer = RAYTRACE;
	glm::vec3 lightPosition = { 0, 0.5, 0.25 };
	bool hasParametersChanged = true;
	if (!lighting.usePhong) Raytrace::preprocessGouraud(objects, lightPosition, camera.cameraPosition, hasParametersChanged);

	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window, camera, renderer, lightPosition, hasParametersChanged);
		if (renderer == RAYTRACE) {
			if (!lighting.usePhong && hasParametersChanged) {
				Raytrace::preprocessGouraud(objects, lightPosition, camera.cameraPosition, hasParametersChanged);
			}
			auto colorBuffer = getRaytrace(camera, objects, textures, lightPosition);
			/*if (lighting.useSoftShadow) {
				applyFilter(colorBuffer);
			}*/
			renderBuffer(colorBuffer, window);
		}
		else drawInterpolationRenders(window, camera, objects, renderer, textures);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
