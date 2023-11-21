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

//Colour globalAmbientColor(70, 20, 20);
Colour globalAmbientColor(20, 20, 20);
Colour globalLightColor(255, 255, 255);
LightOptions lighting( true, true, false, false, true, false);

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

glm::vec2 getLightAttributes(glm::vec3 normal, glm::vec3 lightPosition, glm::vec3 cameraPosition, glm::vec3 position) {
	glm::vec2 output;
	glm::vec3 lightDirection = glm::normalize(lightPosition - position);
	float lightDistance = glm::distance(lightPosition, position);

	float proximityComponent = 1;
	if (lighting.useProximity) {
		float lightIntensity = 5;
		proximityComponent = lightIntensity / (4 * glm::pi<float>() * glm::pow(lightDistance, 2));
	}

	float incidentComponent = 1;
	if (lighting.useIncidence) {
		//incidentComponent = glm::max(glm::dot(normal, lightDirection)*3, 0.0f);
		incidentComponent = glm::dot(normal, lightDirection)*0.5;
	}
	float diffuseComponent = proximityComponent * incidentComponent; // diffuse

	float specularComponent = 0;
	if (lighting.useSpecular) {
		glm::vec3 reflection = glm::reflect(-lightDirection, normal);
		float specularity = glm::max(glm::dot(reflection, glm::normalize(cameraPosition - position)), 0.0f);
		float shininess = 128;
		specularComponent = glm::pow(specularity, shininess);
	}
	return { diffuseComponent, specularComponent };
}

void usePhong(DrawingWindow& window, Camera& camera, PolygonData& objects, glm::vec3 lightPosition) {
	window.clearPixels();
	glm::mat3 inverseViewMatrix = glm::inverse(camera.lookAt({ 0,1,0 }));
	//glm::mat3 inverseViewMatrix = glm::inverse(camera.lookAt({ 0,0,0 }));
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
			glm::vec3 barycentric = intersection.barycentric;
			RayTriangleIntersection shadowIntersection = Raytrace::getClosestValidIntersection(offsetPoint, lightDirection, objects, intersection.triangleIndex, lightDistance);
			if (lighting.useShadow && shadowIntersection.triangleIndex != -1) {
				window.setPixelColour(x, y, lighting.useAmbience ? globalAmbientColor.asNumeric() : 0);
				continue;
			}
			auto vertices = intersection.intersectedTriangle.vertices;

			glm::vec3 interpolatedNormal = glm::normalize(objects.loadedVertices[vertices[0]].normal * barycentric[2] +
				objects.loadedVertices[vertices[1]].normal * barycentric[0] +
				objects.loadedVertices[vertices[2]].normal * barycentric[1]);

			glm::vec2 lightAttributes = getLightAttributes(interpolatedNormal, lightPosition, camera.cameraPosition, intersection.intersectionPoint);
			Colour finalColor = lighting.useAmbience ? globalAmbientColor : Colour(0, 0, 0);
			finalColor = finalColor + intersection.intersectedTriangle.colour * lightAttributes.x +
				globalLightColor * lightAttributes.y;
			window.setPixelColour(x, y, finalColor.asNumeric());
			window.renderFrame();
		}
	}
}

void useGouraud(DrawingWindow& window, Camera& camera, PolygonData& objects, TextureMap& textures, glm::vec3 lightPosition) {
	window.clearPixels();
	//glm::mat3 inverseViewMatrix = glm::inverse(camera.lookAt({ 0,1,0 }));
	glm::mat3 inverseViewMatrix = glm::inverse(camera.lookAt({ 0,0,0 }));
	//for (int y = 0; y < HEIGHT; y++) {
	for (int y = HEIGHT - 1; y > -1; y--) {
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
			if (lighting.useShadow && shadowIntersection.triangleIndex != -1) {
				window.setPixelColour(x, y, lighting.useAmbience ? globalAmbientColor.asNumeric() : 0);
				continue;
			}

			auto vertices = intersection.intersectedTriangle.vertices;
			glm::vec3 barycentric = intersection.barycentric;
			float cameraDistance = intersection.distanceFromCamera;
			Colour baseColor = intersection.intersectedTriangle.colour;
			Colour finalColor;

			// apply texture map if necessary
			if (intersection.intersectedTriangle.texturePoints[0] != -1) {
				std::array<glm::vec2, 3> textureVertices = objects.getTextureVertices(intersection.triangleIndex);
				GouraudVertex vertex1 = objects.loadedVertices[vertices[0]];
				GouraudVertex vertex2 = objects.loadedVertices[vertices[1]];
				GouraudVertex vertex3 = objects.loadedVertices[vertices[2]];
				textureVertices[0] /= cameraDistance;
				textureVertices[1] /= cameraDistance;
				textureVertices[2] /= cameraDistance;
				float interpolatedDepth = barycentric[0] / glm::abs(cameraDistance) 
					+ barycentric[1] / glm::abs(cameraDistance) 
					+ barycentric[2] / glm::abs(cameraDistance);
				glm::vec2 coordinate = barycentric[0] * textureVertices[0] 
					+ barycentric[1] * textureVertices[1]  
					+ barycentric[2] * textureVertices[2];
				coordinate *= (1 / interpolatedDepth);
				baseColor = Colour(textures.pixels[glm::floor(glm::max(coordinate.x, 0.0f)) +
					glm::floor(glm::max(coordinate.y, 0.0f)) * textures.width
				]);

				glm::vec2 weight1 = barycentric[2] * getLightAttributes(
					vertex1.normal, lightPosition, camera.cameraPosition, vertex1.position);
				glm::vec2 weight2 = barycentric[0] * getLightAttributes(
					vertex2.normal, lightPosition, camera.cameraPosition, vertex2.position);
				glm::vec2 weight3 = barycentric[1] * getLightAttributes(
					vertex3.normal, lightPosition, camera.cameraPosition, vertex3.position);

				glm::vec2 finalAttributes = (weight1 + weight2 + weight3);
				finalColor = globalAmbientColor + baseColor * finalAttributes.x /*+ globalLightColor * finalAttributes.y*/ ;
			}
			else {
				// apply interpolated lighting from each vertex
				finalColor = 
					objects.loadedVertices[vertices[0]].renderedColor * barycentric[2] +
					objects.loadedVertices[vertices[1]].renderedColor * barycentric[0] +
					objects.loadedVertices[vertices[2]].renderedColor * barycentric[1];
			}

			window.setPixelColour(x, y, finalColor.asNumeric());
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

void preprocessGouraud(PolygonData& objects, glm::vec3 lightPosition, glm::vec3 cameraPosition, bool& hasParametersChanged) {
	hasParametersChanged = false;
	for (auto& vertex : objects.loadedVertices) {
		glm::vec3 lightDirection = glm::normalize(lightPosition - glm::vec3(vertex));
		float lightDistance = glm::distance(lightPosition, glm::vec3(vertex));

		if (lighting.useAmbience) vertex.ambient = globalAmbientColor;

		auto attributes = getLightAttributes(vertex.normal, lightPosition,
			cameraPosition, glm::vec3(vertex));

		vertex.diffuse = vertex.originalColor * attributes.x;
		vertex.specular = globalLightColor * attributes.y;

		vertex.renderedColor = (vertex.ambient + vertex.diffuse + vertex.specular);
		std::cout << vertex << std::endl;

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
	//Camera camera(0.0, 4.5, 6);

	FileReader fr;
	fr.readMTLFile("textured-cornell-box.mtl");
	//fr.readMTLFile("cornell-box.mtl");
	PolygonData objects = fr.readOBJFile("textured-cornell-box.obj", 0.35, { textures.width, textures.height });
	//PolygonData objects = fr.readOBJFile("cornell-box.obj", 0.35, {textures.width, textures.height});
	//PolygonData objects = fr.readOBJFile("sphere.obj", 1);
	if (objects.loadedTriangles.empty()) return -1;
	
	// calculate normals
	for (int triangleIndex = 0; triangleIndex < objects.loadedTriangles.size(); triangleIndex++) {
		glm::vec3 e0 = objects.getTriangleVertexPosition(triangleIndex, 1)
			- objects.getTriangleVertexPosition(triangleIndex, 0);
		glm::vec3 e1 = objects.getTriangleVertexPosition(triangleIndex, 2) -
			objects.getTriangleVertexPosition(triangleIndex, 0);
		objects.loadedTriangles[triangleIndex].normal = glm::normalize(glm::cross(e0, e1)); // winding order for cornell box
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

	RenderType renderer = RAYTRACE;
	glm::vec3 lightPosition = { 0, 0.5, 0.25 };
	//glm::vec3 lightPosition = { 0.5, 2.5, 2 };
	//glm::vec3 lightPosition = { 0.5, 2.5, 2 };
	bool hasParametersChanged = true;
	if (!lighting.usePhong) preprocessGouraud(objects, lightPosition, camera.cameraPosition, hasParametersChanged);

	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window, camera, renderer, lightPosition, hasParametersChanged);
		if (renderer == RAYTRACE) {
			if (!lighting.usePhong) {
				if (hasParametersChanged) preprocessGouraud(objects, lightPosition, camera.cameraPosition, hasParametersChanged);
				useGouraud(window, camera, objects, textures, lightPosition);
			}
			else {
				usePhong(window, camera, objects, lightPosition);
			}
		}
		else drawInterpolationRenders(window, camera, objects, renderer, textures);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
