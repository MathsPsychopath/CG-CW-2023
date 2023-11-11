#include "Rasterize.h"

namespace {
	// projects the left vertex and returns all 4 top, left, right, bottom
	std::array<CanvasPoint, 4> projectLeftVertex(const std::array<CanvasPoint, 3>& sortedVertices) {
		// Compute projection, which is the linear interpolation formula rearranged for x
		float xProjection = sortedVertices[0].x
			+ ((sortedVertices[1].y - sortedVertices[0].y) / (sortedVertices[2].y - sortedVertices[0].y))
			* (sortedVertices[2].x - sortedVertices[0].x);

		CanvasPoint projectedPoint = CanvasPoint(std::round(xProjection), sortedVertices[1].y);

		// decide the left most vertex
		CanvasPoint left = projectedPoint.x < sortedVertices[1].x ? projectedPoint : sortedVertices[1];
		CanvasPoint right = left.x == projectedPoint.x ? sortedVertices[1] : projectedPoint;

		std::array<CanvasPoint, 4> output = { sortedVertices[0], left, right, sortedVertices[2] };
		return output;
	}

	// interpolates single floats
	std::vector<float> singleFloat(float from, float to, int numberOfValues) {
		const float increment = (to - from) / (numberOfValues - 1);
		std::vector<float> output = {};
		for (int i = 0; i < numberOfValues; i++) {
			output.push_back(from + i * increment);
		}
		return output;
	}

	// sorts vertices by y-value ascending
	void sortTriangle(std::array<CanvasPoint, 3>& vertices) {
		if (vertices[0].y > vertices[1].y) {
			std::swap(vertices[0], vertices[1]);
		}
		if (vertices[1].y > vertices[2].y) {
			std::swap(vertices[1], vertices[2]);
		}
		if (vertices[0].y > vertices[1].y) {
			std::swap(vertices[0], vertices[1]);
		}
	}

	// gets the specific texture pixel given map and relative coordinate of original
	uint32_t getTexture(BarycentricCoordinates coordinates, std::array<CanvasPoint, 3> sortedVertices, TextureMap textures) {
		int width = textures.width;
		int height = textures.height;
		glm::vec2 textureA(sortedVertices[0].texturePoint.x, sortedVertices[0].texturePoint.y);
		glm::vec2 textureB(sortedVertices[1].texturePoint.x, sortedVertices[1].texturePoint.y);
		glm::vec2 textureC(sortedVertices[2].texturePoint.x, sortedVertices[2].texturePoint.y);
		glm::vec2 textureCoordinate =
			textureA * coordinates.A +
			textureB * coordinates.B +
			textureC * coordinates.C;

		return textures.pixels[std::floor(textureCoordinate.x) + std::floor(textureCoordinate.y) * width];
	}
}

std::vector<glm::vec3> Rasterize::threeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValues) {
	const int indices = numberOfValues - 1;
	glm::vec3 steps((to.x - from.x) / indices, (to.y - from.y) / indices, (to.z - from.z) / indices);
	std::vector<glm::vec3> output = {};
	for (int i = 0; i < numberOfValues; i++) {
		output.push_back(from + (steps * float(i)));
	}
	return output;
}

InterpolatedTriangle Rasterize::triangle(const std::array<CanvasPoint, 3>& sortedVertices) {

	std::array<CanvasPoint, 4> vertices = projectLeftVertex(sortedVertices);

	InterpolatedTriangle output;

	// interpolate x values for edges
	output.topLeft = singleFloat(vertices[0].x, vertices[1].x, std::abs(vertices[1].y - vertices[0].y) + 1);
	output.topRight = singleFloat(vertices[0].x, vertices[2].x, std::abs(vertices[2].y - vertices[0].y) + 1);
	output.leftBottom = singleFloat(vertices[1].x, vertices[3].x, std::abs(vertices[3].y - vertices[1].y) + 1);
	output.rightBottom = singleFloat(vertices[2].x, vertices[3].x, std::abs(vertices[3].y - vertices[2].y) + 1);

	return output;
}

void Rasterize::drawRasterizedTriangle(DrawingWindow& window, CanvasTriangle triangle, Colour color, std::vector<std::vector<float>>& zDepth) {
	// translate vertices to interface type
	std::array<CanvasPoint, 3> vertices = { triangle.v0(), triangle.v1(), triangle.v2() };

	// sort triangle and interpolate
	sortTriangle(vertices);
	InterpolatedTriangle interpolations = Rasterize::triangle(vertices);

	// rasterize top triangle
	uint32_t pixelColor = (255 << 24) + (int(color.red) << 16) + (int(color.green) << 8) + int(color.blue);
	for (int y = std::floor(vertices[0].y), i = 0; y < std::floor(vertices[1].y); y++, i++) {
		if (y >= HEIGHT || y < 0) continue;
		int xStart = std::floor(interpolations.topLeft[i]);
		int xEnd = std::ceil(interpolations.topRight[i]);
		if (std::abs(xStart - xEnd) < 2) continue;
		for (int x = xStart; x < xEnd; x++) {
			if (x < 0 || x >= WIDTH) continue;
			// use barycentric ratios to calculate zIndex
			BarycentricCoordinates ratios = barycentric(vertices, glm::vec2(x, y));
			float zIndex = 1 / (ratios.A * vertices[0].depth + ratios.B * vertices[1].depth + ratios.C * vertices[2].depth);
			if (zDepth[y][x] < zIndex) continue;
			window.setPixelColour(x, y, pixelColor);
			zDepth[y][x] = zIndex;
		}
	}
	// rasterize bottom triangle
	for (int y = std::floor(vertices[1].y), i = 0; y < std::floor(vertices[2].y); y++, i++) {
		if (y >= HEIGHT || y < 0) continue;
		int xStart = std::floor(interpolations.leftBottom[i]);
		int xEnd = std::ceil(interpolations.rightBottom[i]);
		if (std::abs(xStart - xEnd) < 2) continue;
		for (int x = xStart; x < xEnd; x++) {
			if (x < 0 || x >= WIDTH) continue;
			// interpolate z values
			BarycentricCoordinates ratios = barycentric(vertices, glm::vec2(x, y));
			float zIndex = 1 / (ratios.A * vertices[0].depth + ratios.B * vertices[1].depth + ratios.C * vertices[2].depth);
			if (zDepth[y][x] < zIndex) continue;
			window.setPixelColour(x, y, pixelColor);
			zDepth[y][x] = zIndex;
		}
	}
}

void Rasterize::drawRasterizedTriangle(DrawingWindow& window, CanvasTriangle triangle, TextureMap textures, std::vector<std::vector<float>>& zDepth) {
	// translate vertices to interface type
	std::array<CanvasPoint, 3> canvasVertices = { triangle.v0(), triangle.v1(), triangle.v2() };

	// sort canvas triangle and interpolate
	sortTriangle(canvasVertices);
	std::cout << canvasVertices[0] << canvasVertices[1] << canvasVertices[2] << std::endl;
	InterpolatedTriangle interpolations = Rasterize::triangle(canvasVertices);

	// rasterize top triangle with textures
	for (int y = canvasVertices[0].y, i = 0; y < canvasVertices[1].y; y++, i++) {
		for (int x = std::floor(interpolations.topLeft[i]); x < std::ceil(interpolations.topRight[i]); x++) {
			if (y < 0 || x < 0 || y >= HEIGHT || x >= WIDTH) continue;
			glm::vec2 currentVertex(x, y);
			BarycentricCoordinates ratios = barycentric(canvasVertices, currentVertex);
			uint32_t pixelTexture = getTexture(ratios, canvasVertices, textures);
			float zIndex = 1 / (ratios.A * canvasVertices[0].depth + ratios.B * canvasVertices[1].depth + ratios.C * canvasVertices[2].depth);
			if (zDepth[y][x] < zIndex) continue;
			window.setPixelColour(x, y, pixelTexture);
			zDepth[y][x] = zIndex;
		}
	}

	// rasterize bottom triangle with textures
	for (int y = canvasVertices[1].y, i = 0; y < canvasVertices[2].y; y++, i++) {
		for (int x = std::floor(interpolations.leftBottom[i]); x < std::ceil(interpolations.rightBottom[i]); x++) {
			if (y < 0 || x < 0 || y >= HEIGHT || x >= WIDTH) continue;
			glm::vec2 currentVertex(x, y);
			BarycentricCoordinates ratios = barycentric(canvasVertices, currentVertex);
			uint32_t pixelTexture = getTexture(ratios, canvasVertices, textures);
			float zIndex = 1 / (ratios.A * canvasVertices[0].depth + ratios.B * canvasVertices[1].depth + ratios.C * canvasVertices[2].depth);
			if (zDepth[y][x] < zIndex) continue;
			window.setPixelColour(x, y, pixelTexture);
			zDepth[y][x] = zIndex;
		}
	}
}

BarycentricCoordinates Rasterize::barycentric(const std::array<CanvasPoint, 3>& sortedVertices, glm::vec2 encodedVertex) {
	CanvasPoint A = sortedVertices[0];
	CanvasPoint B = sortedVertices[1];
	CanvasPoint C = sortedVertices[2];
	BarycentricCoordinates output = {};

	// cache the denominator as it is the same value for both
	float denominator = ((B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y));

	// calculate the ratio of distances from each vertex
	// Area of PBC / Area of ABC
	output.A = ((B.y - C.y) * (encodedVertex.x - C.x) + (C.x - B.x) * (encodedVertex.y - C.y)) / denominator;

	// Area of APC / Area of ABC
	output.B = ((C.y - A.y) * (encodedVertex.x - C.x) + (A.x - C.x) * (encodedVertex.y - C.y)) / denominator;

	output.C = 1 - output.A - output.B;

	return output;
}