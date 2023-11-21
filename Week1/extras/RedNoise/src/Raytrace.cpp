#include "Raytrace.h"

//Colour globalAmbientColor(70, 20, 20);
Colour globalAmbientColor(20, 20, 20);
Colour globalLightColor(255, 255, 255);

namespace {

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
			incidentComponent = glm::dot(normal, lightDirection) * 0.5;
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
}

RayTriangleIntersection Raytrace::getClosestValidIntersection(glm::vec3 cameraPosition, glm::vec3 rayDirection, PolygonData& objects, int excludeID, float lightDistance) {
	RayTriangleIntersection closest;
	closest.distanceFromCamera = std::numeric_limits<float>::max();
	closest.triangleIndex = -1;
	for (int triangleIndex = 0; triangleIndex < objects.loadedTriangles.size(); triangleIndex++) {
		if (triangleIndex == excludeID) continue;
		glm::vec3 e0 = objects.getTriangleVertexPosition(triangleIndex, 1) - objects.getTriangleVertexPosition(triangleIndex, 0);
		glm::vec3 e1 = objects.getTriangleVertexPosition(triangleIndex, 2) - objects.getTriangleVertexPosition(triangleIndex, 0);
		glm::vec3 SPVector = cameraPosition - objects.getTriangleVertexPosition(triangleIndex, 0);
		glm::mat3 DEMatrix(-rayDirection, e0, e1);
		glm::vec3 possibleSolution = glm::inverse(DEMatrix) * SPVector;
		float t = possibleSolution.x; // distance from camera
		float u = possibleSolution.y; // distance along v1-v0 edge
		float v = possibleSolution.z; // distance along v2-v0 edge
		
		// assert validity check
		if (u >= 0.0 && u <= 1.0 && v >= 0.0 && v <= 1.0 && u + v <= 1.0) {
			// get the closest triangle to camera
			if (t > lightDistance || t > closest.distanceFromCamera || t < 0) {
				continue;
			}
			closest.distanceFromCamera = t;
			closest.triangleIndex = triangleIndex;
			closest.intersectedTriangle = objects.loadedTriangles[triangleIndex];
			closest.intersectionPoint = cameraPosition + t * rayDirection;
			closest.barycentric = glm::vec3{ u, v, 1 - (u + v) };
		}
	}
	return closest;
}

glm::vec3 Raytrace::getCanvasPosition(Camera& camera, CanvasPoint position, glm::mat3 inverseViewMatrix) {
	int scaleFactor = 90;
	float focalLength = 2.0f;
	position.x = WIDTH - position.x;
	float realX = ((position.x - WIDTH / 2) / (scaleFactor * focalLength));
	float realY = ((position.y - HEIGHT / 2) / (scaleFactor * focalLength));
	glm::vec3 displacement = glm::vec3(realX, realY, focalLength) * inverseViewMatrix;
	return camera.cameraPosition + displacement;
}

void Raytrace::useGouraud(DrawingWindow& window, Camera& camera, PolygonData& objects, TextureMap& textures, glm::vec3 lightPosition) {
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
				finalColor = globalAmbientColor + baseColor * finalAttributes.x /*+ globalLightColor * finalAttributes.y*/;
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

void Raytrace::preprocessGouraud(PolygonData& objects, glm::vec3 lightPosition, glm::vec3 cameraPosition, bool& hasParametersChanged) {
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

void Raytrace::usePhong(DrawingWindow& window, Camera& camera, PolygonData& objects, glm::vec3 lightPosition) {
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
