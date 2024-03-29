#include "Raytrace.h"

//Colour globalAmbientColor(70, 20, 20);
Colour globalAmbientColor(20, 20, 20);
Colour globalLightColor(255, 255, 255);

namespace {

	glm::vec2 getLightAttributes(glm::vec3& normal, glm::vec3& lightPosition, glm::vec3& start, glm::vec3& position) {
		glm::vec2 output;
		glm::vec3 lightDirection = glm::normalize(lightPosition - position);
		float lightDistance = glm::distance(lightPosition, position);

		float proximityComponent = 1;
		if (lighting.useProximity) {
			float lightIntensity = 10;
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
			float specularity = glm::max(glm::dot(reflection, glm::normalize(start - position)), 0.0f);
			float shininess = 128;
			specularComponent = glm::pow(specularity, shininess);
		}
		return { diffuseComponent, specularComponent };
	}

	bool intersectsBoundingBox(ModelTriangle& triangle, const glm::vec3 invertedDir, const glm::vec3 origin) {
		// AABB bounding box intersection algorithm
		glm::vec3 rayMin = (triangle.boundingMinMax.first - origin) * invertedDir;
		glm::vec3 rayMax = (triangle.boundingMinMax.second - origin) * invertedDir;
		for (int i = 0; i < 3; i++) {
			if (invertedDir[i] < 0.0f) std::swap(rayMin[i], rayMax[i]);
		}
		float finalEntry = glm::max(rayMin.x, rayMin.y, rayMin.z);
		float finalExit = glm::min(rayMax.x, rayMax.y, rayMax.z);
		if (finalExit < finalEntry || finalExit < 0) return false;
		return true;
	}

	RayTriangleIntersection getClosestValidIntersection(glm::vec3& startPosition, glm::vec3& rayDirection, PolygonData& objects, std::set<std::string>& hiddenObjects, int excludeID = -1, float lightDistance = std::numeric_limits<float>::max()) {
		RayTriangleIntersection closest;
		closest.distanceFromCamera = std::numeric_limits<float>::max();
		closest.triangleIndex = -1;
		glm::vec3 invertedDirection = 1.0f / rayDirection;
		for (int triangleIndex = 0; triangleIndex < objects.loadedTriangles.size(); triangleIndex++) {
			if (triangleIndex == excludeID) continue;
			if (hiddenObjects.find(objects.loadedTriangles[triangleIndex].objectName) != hiddenObjects.end()) continue;
			if (!intersectsBoundingBox(objects.loadedTriangles[triangleIndex], invertedDirection, startPosition)) {
				continue;
			}
			glm::vec3 e0 = objects.getTriangleVertexPosition(triangleIndex, 1) - objects.getTriangleVertexPosition(triangleIndex, 0);
			glm::vec3 e1 = objects.getTriangleVertexPosition(triangleIndex, 2) - objects.getTriangleVertexPosition(triangleIndex, 0);
			glm::vec3 SPVector = startPosition - objects.getTriangleVertexPosition(triangleIndex, 0);
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
				closest.intersectionPoint = startPosition + t * rayDirection;
				closest.barycentric = glm::vec3{ u, v, 1 - (u + v) };
			}
		}
		return closest;
	}

	// inverse of getCanvasIntersection
	glm::vec3 getCanvasPosition(Camera& camera, int x, int y, glm::mat3& inverseViewMatrix) {
		int scaleFactor = 90;
		float focalLength = 2.0f;
		x = WIDTH - x;
		float realX = ((x - WIDTH / 2) / (scaleFactor * focalLength));
		float realY = ((y - HEIGHT / 2) / (scaleFactor * focalLength));
		glm::vec3 displacement = glm::vec3(realX, realY, focalLength) * inverseViewMatrix;
		return camera.cameraPosition + displacement;
	}
	
	float getSoftShadow(PolygonData& objects, RayTriangleIntersection& initialIntersection, glm::vec3& lightPosition, glm::vec3 cameraPosition, std::set<std::string>& hiddenObjects) {
		// check they're on the same side
		glm::vec3 normal = initialIntersection.intersectedTriangle.normal;
		glm::vec3 offset = initialIntersection.intersectionPoint + 0.01f * normal;
		glm::vec3 cameraDirection = glm::normalize(cameraPosition - offset); // point to camera

		float cameraNormalAngle = glm::dot(normal, cameraDirection);
		if (cameraNormalAngle < 0) return 1;

		int samples = 40; // increase for better shadows, worse performance
		float lightRadius = 0.1;
		glm::vec3 offsetPoint = initialIntersection.intersectionPoint + 
			0.01f * initialIntersection.intersectedTriangle.normal;
		int hits = 0;
		for (int i = 0; i < samples; i++) {
			glm::vec3 sampledLight = Lighting::sampleLightPosition(lightPosition, lightRadius);
			glm::vec3 direction = glm::normalize(sampledLight - offsetPoint);
			float lightDistance = glm::length(sampledLight - offsetPoint);
			RayTriangleIntersection shadowIntersection = 
				getClosestValidIntersection(offsetPoint, direction, objects, hiddenObjects, initialIntersection.triangleIndex, lightDistance);
			if (shadowIntersection.triangleIndex == -1) hits += 1;
		}

		return float(hits) / samples;
	}

	glm::vec3 getPhongNormal(PolygonData& objects, RayTriangleIntersection& intersection) {
		std::array<int, 3> vertices = intersection.intersectedTriangle.vertices;
		glm::vec3 barycentric = intersection.barycentric;
		glm::vec3 interpolatedNormal = glm::normalize(objects.loadedVertices[vertices[0]].normal * barycentric[2] +
			objects.loadedVertices[vertices[1]].normal * barycentric[0] +
			objects.loadedVertices[vertices[2]].normal * barycentric[1]);
		return interpolatedNormal;
	}

	glm::vec2 calculatePhongComponents(PolygonData& objects, RayTriangleIntersection& intersection, glm::vec3 lightPosition, glm::vec3 start) {
		// interpolate the normal by the pixel
		glm::vec3 interpolatedNormal = getPhongNormal(objects, intersection);
		glm::vec3 pixelCoordinates = intersection.intersectionPoint;
		return getLightAttributes(interpolatedNormal, lightPosition, start, pixelCoordinates);
	}

	std::vector<glm::vec2> calculateGouraudComponents(PolygonData& objects, RayTriangleIntersection& intersection) {
		int triangleIndex = intersection.triangleIndex;
		glm::vec3 barycentric = intersection.barycentric;
		GouraudVertex vertex1 = objects.getTriangleVertex(triangleIndex, 0);
		GouraudVertex vertex2 = objects.getTriangleVertex(triangleIndex, 1);
		GouraudVertex vertex3 = objects.getTriangleVertex(triangleIndex, 2);
		glm::vec2 v1Components = { vertex1.diffuse, vertex1.specular };
		glm::vec2 v2Components = { vertex2.diffuse, vertex2.specular };
		glm::vec2 v3Components = { vertex3.diffuse, vertex3.specular };
		return { v1Components * barycentric[2], v2Components * barycentric[0], v3Components * barycentric[1] };
	}

	Colour getRaytracedTexture(PolygonData& objects, RayTriangleIntersection& intersection, TextureMap& textures) {
		std::array<glm::vec2, 3> textureVertices = objects.getTextureVertices(intersection.triangleIndex);
		int triangleIndex = intersection.triangleIndex;
		std::array<int, 3> vertices = objects.loadedTriangles[triangleIndex].vertices;
		float cameraDistance = intersection.distanceFromCamera;
		glm::vec3 barycentric = intersection.barycentric;
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
		return Colour(textures.pixels[glm::floor(glm::max(coordinate.x, 0.0f)) +
			glm::floor(glm::max(coordinate.y, 0.0f)) * textures.width
		]);
	}

	std::pair<Colour, RayTriangleIntersection> raytrace(PolygonData& objects, TextureMap& textures, glm::vec3 start, glm::vec3 direction, glm::vec3 lightOrigin, Camera& camera, std::set<std::string>& hiddenObjects) {
		// get initial ray trace
		RayTriangleIntersection intersection = getClosestValidIntersection(start, direction, objects, hiddenObjects);
		if (intersection.triangleIndex == -1) {
			return { Colour(), intersection };
		}

		// conditionally apply hard shadows
		if (lighting.useShadow) {
			// check they're on the same side
			glm::vec3 normal = intersection.intersectedTriangle.normal;
			glm::vec3 offsetPoint = intersection.intersectionPoint + 0.01f * normal;

			glm::vec3 cameraDirection = glm::normalize(camera.cameraPosition - offsetPoint); // point to camera
			glm::vec3 lightDirection = glm::normalize(lightOrigin - offsetPoint);
			float cameraNormalAngle = glm::dot(normal, cameraDirection);
			float lightNormalAngle = glm::dot(normal, lightDirection);
			if (cameraNormalAngle > 0 || cameraNormalAngle > 0 && lightNormalAngle < 0) {
				float lightDistance = glm::length(lightOrigin - camera.cameraPosition);
				RayTriangleIntersection shadowIntersection =
					getClosestValidIntersection(offsetPoint, lightDirection, objects, hiddenObjects, intersection.triangleIndex, lightDistance);

				if (shadowIntersection.triangleIndex != -1) {
					RayTriangleIntersection hardShadow(glm::vec3{ 0,0,0 }, 0, ModelTriangle(), -1);
					return { lighting.useAmbience ? globalAmbientColor : Colour(), hardShadow };
				}
			}
		}
		// conditionally get texture map as pixel color, which needs on-the-fly getLightAttribute.
		Colour baseColor = intersection.intersectedTriangle.colour;
		if (intersection.intersectedTriangle.texturePoints[0] != -1) {
			baseColor = getRaytracedTexture(objects, intersection, textures);
		}
		// diverge between phong and gouraud shading and calculate diffuse & specular components
		Colour diffuse = baseColor;
		Colour specular = globalLightColor;
		if (lighting.usePhong) {
			glm::vec2 lightingComponents = calculatePhongComponents(objects, intersection, lightOrigin, start);
			diffuse *= lightingComponents.x;
			specular *= lightingComponents.y;
		}
		else {
			std::vector<glm::vec2> lightingComponents = calculateGouraudComponents(objects, intersection);
			diffuse =
				baseColor * lightingComponents[0].x +
				baseColor * lightingComponents[1].x +
				baseColor * lightingComponents[2].x;
			specular = globalLightColor * lightingComponents[0].y +
				globalLightColor * lightingComponents[1].y +
				globalLightColor * lightingComponents[2].y;
		}

		float brightness = 1;
		if (lighting.useSoftShadow) {
			brightness = getSoftShadow(objects, intersection, lightOrigin, camera.cameraPosition, hiddenObjects);
		}

		Colour ambience = lighting.useAmbience ? globalAmbientColor : Colour();
		// apply shading to color
		Colour finalColor = (ambience + diffuse + specular) * brightness;
		return { finalColor, intersection };
	}
}

void Raytrace::renderSegment(glm::vec2 boundY, std::vector<std::vector<uint32_t>>& colorBuffer, PolygonData& objects, Camera& camera, TextureMap& textures, glm::vec3 lightOrigin, std::set<std::string>& hiddenObjects) {
	glm::mat3 inverseViewMatrix = glm::inverse(camera.viewMatrix);
	for (int y = boundY[0]; y < boundY[1]; y++) {
		for (int x = 0; x < WIDTH; x++) {
			if (x == WIDTH / 4 * 3 && y == HEIGHT / 2) {
				std::cout << "here" << std::endl;
			}
			// get point on the ray trace
			glm::vec3 canvasPosition = getCanvasPosition(camera, x, y, inverseViewMatrix);
			glm::vec3 direction = glm::normalize(camera.cameraPosition - canvasPosition);


			auto colorTrianglePair = raytrace(objects, textures, camera.cameraPosition, direction, lightOrigin, camera, hiddenObjects);

			Colour color = colorTrianglePair.first;
			RayTriangleIntersection intersection = colorTrianglePair.second;

			if (intersection.triangleIndex == -1) {
				colorBuffer[y][x] = color.asNumeric();
				continue;
			}

			float reflectivity = intersection.intersectedTriangle.reflectivity;
			// conditionally apply reflectiveness 
			if (lighting.useReflections && std::isgreater(reflectivity, 0)) {
				// isolate normal interpolation function
				
				glm::vec3 normal = lighting.usePhong ? getPhongNormal(objects, intersection) : intersection.intersectedTriangle.normal;
				// calculate reflection ray
				glm::vec3 reflectionRay = glm::reflect(direction, normal);
				// raytrace from intersection point in the direction of the reflection
				glm::vec3 offsetPoint = intersection.intersectionPoint + 0.01f * normal;
				auto reflectionPair = raytrace(objects, textures, offsetPoint, reflectionRay, lightOrigin, camera, hiddenObjects);
				color = color * (1 - reflectivity) + reflectionPair.first * reflectivity;
			}
			colorBuffer[y][x] = color.asNumeric();
		}
	}
}

void Raytrace::preprocessGouraud(PolygonData& objects, glm::vec3& lightPosition, glm::vec3& cameraPosition) {
	for (auto& vertex : objects.loadedVertices) {
		glm::vec3 lightDirection = glm::normalize(lightPosition - glm::vec3(vertex));
		float lightDistance = glm::distance(lightPosition, glm::vec3(vertex));

		if (lighting.useAmbience) vertex.ambient = globalAmbientColor;

		auto attributes = getLightAttributes(vertex.normal, lightPosition,
			cameraPosition, vertex.position);

		vertex.diffuse = attributes.x;
		vertex.specular = attributes.y;
	}
}

