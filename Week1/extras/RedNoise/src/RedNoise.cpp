#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include "FileReader.h"
#include <glm/gtx/string_cast.hpp>
#include "Rasterize.h"
#include "Wireframe.h"
#include "Raytrace.h"

void drawInterpolationRenders(DrawingWindow& window, Camera &camera, PolygonData& objects, RenderType type, TextureMap& textures, std::set<std::string>& hiddenObjects) {
	window.clearPixels();
	glm::mat3 viewMatrix = camera.viewMatrix;
	std::vector<std::vector<float>> zDepth(HEIGHT, std::vector<float>(WIDTH, std::numeric_limits<float>::max()));
	for (int triangleIndex = 0; triangleIndex < objects.loadedTriangles.size(); triangleIndex++) {
		if (hiddenObjects.find(objects.loadedTriangles[triangleIndex].objectName) != hiddenObjects.end()) continue;
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

std::vector<std::vector<uint32_t>> getRaytrace(Camera& camera, PolygonData& objects, TextureMap& textures, glm::vec3 lightPosition, std::set<std::string>& hiddenObjects) {
	// create results canvas
	std::vector<std::vector<uint32_t>> colorBuffer(HEIGHT, std::vector<uint32_t>(WIDTH));
	std::vector<std::thread> threads;
	// parallelise workload
	
	for (int i = 0; i < 4; i++) {
		int startY = (HEIGHT >> 2) * i;
		int endY = (HEIGHT >> 2) * (i + 1);
		threads.emplace_back(Raytrace::renderSegment, glm::vec2{startY, endY}, std::ref(colorBuffer), std::ref(objects), std::ref(camera), std::ref(textures), lightPosition, std::ref(hiddenObjects));
	}
	for (auto& thread : threads) thread.join();
	return colorBuffer;
}

float useGaussian(float value, float stddev) {
	return std::expf(-(glm::pow(value, 2) / (2.0f * glm::pow(stddev, 2))));
}

void splitChannels(uint32_t packed, int& red, int& green, int& blue) {
	red = (packed >> 16) & 0xff;
	green = (packed >> 8) & 0xff;
	blue = (packed) & 0xff;
}

uint32_t packChannels(int red, int green, int blue) {
	return (255 << 24) + (red << 16) + (green << 8) + blue;
}

void useBilteralFilter(glm::vec2 boundY, std::vector<std::vector<uint32_t>>& colorBuffer, std::vector<std::vector<uint32_t>>& output) {
	// apply bilateral filter algorithm
	float sigmaSpace = 2;
	float sigmaRange = 17;
	int radius = int(2 * sigmaSpace);
	
	for (int y = boundY[0]; y < boundY[1]; y++) {
		for (int x = 0; x < WIDTH; x++) {
			float sumOfWeights = 0;
			float responseR = 0;
			float responseG = 0;
			float responseB = 0;

			int currentR, currentG, currentB;
			splitChannels(colorBuffer[y][x], currentR, currentG, currentB);
			for (int dx = -radius; dx < radius; dx++) {
				for (int dy = -radius; dy < radius; dy++) {
					int nx = x + dx;
					int ny = y + dy;

					if (nx >= 0 && nx < WIDTH && ny >= 0 && ny < HEIGHT) {
						int kernelR, kernelG, kernelB;
						splitChannels(colorBuffer[ny][nx], kernelR, kernelG, kernelB);
						float colorDist = glm::sqrt(
							glm::pow(kernelR - currentR, 2) +
							glm::pow(kernelG - currentG, 2) +
							glm::pow(kernelB - currentB, 2)
						);

						float spaceWeight = useGaussian(glm::sqrt(glm::pow(dx, 2) + glm::pow(dy, 2)), sigmaSpace);
						float rangeWeight = useGaussian(colorDist, sigmaRange);
						float weight = spaceWeight * rangeWeight;

						responseR += weight * kernelR;
						responseG += weight * kernelG;
						responseB += weight * kernelB;
						sumOfWeights += weight;
					}
				}
			}
			int finalR = responseR / sumOfWeights;
			int finalG = responseG / sumOfWeights;
			int finalB = responseB / sumOfWeights;
			output[y][x] = packChannels(finalR, finalG, finalB);
		}
	}
}

void applyFilter(std::vector<std::vector<uint32_t>>& colorBuffer) {
	std::vector<std::vector<uint32_t>> output(HEIGHT, std::vector<uint32_t>(WIDTH));
	// parallelise workload here
	std::vector<std::thread> threads;
	for (int i = 0; i < 4; i++) {
		int startY = (HEIGHT >> 2) * i;
		int endY = (HEIGHT >> 2) * (i + 1);
		threads.emplace_back(useBilteralFilter, glm::vec2{ startY, endY }, std::ref(colorBuffer), std::ref(output));
	}
	for (auto& thread : threads) thread.join();
	colorBuffer = output;
}

void renderBuffer(std::vector<std::vector<uint32_t>>& colorBuffer, DrawingWindow& window) {
	for (int y = HEIGHT - 1; y > -1; y--) {
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
			else camera.rotate(0, -1, 0);
		}
		else if (event.key.keysym.sym == SDLK_RIGHT) {
			if (renderer == RAYTRACE) lightPosition += glm::vec3(0.25, 0, 0);
			else camera.rotate(0, 1, 0);
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
	fr.appendPolygonData(objects, "sphere.obj");
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
	glm::vec3 lightPosition = { 0, 0.5, 0.75 };
	bool hasParametersChanged = true;
	if (!lighting.usePhong) Raytrace::preprocessGouraud(objects, lightPosition, camera.cameraPosition, hasParametersChanged);

	bool isCameraMoving = true;
	float progression = 0;
	int stage = 0;
	std::set<std::string> hiddenObjects = {"red_sphere"};
	
	int frame = 0;
	while (isCameraMoving) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window, camera, renderer, lightPosition, hasParametersChanged);
		camera.useAnimation(progression, stage, renderer, hiddenObjects, lighting, isCameraMoving);
		if (renderer == RAYTRACE) {
			if (!lighting.usePhong) {
				Raytrace::preprocessGouraud(objects, lightPosition, camera.cameraPosition, hasParametersChanged);
			}
			auto colorBuffer = getRaytrace(camera, objects, textures, lightPosition, hiddenObjects);
			/*if (lighting.useSoftShadow) {
				applyFilter(colorBuffer);
			}*/
			renderBuffer(colorBuffer, window);
		}
		else drawInterpolationRenders(window, camera, objects, renderer, textures, hiddenObjects);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		std::string frameString = std::to_string(frame++);
		std::string filename = "frame" + std::string(4 - std::min(4, int(frameString.length())), '0') + frameString + ".bmp";
		window.renderFrame();
		window.saveBMP("./renders/" + filename);
		std::cout << "rendered frame " << frame << std::endl;
		if (progression > 1) {
			progression = 0;
			stage++;
		}
	}
}
