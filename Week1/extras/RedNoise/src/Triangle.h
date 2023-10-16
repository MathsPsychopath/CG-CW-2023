#pragma once
#include "DrawingWindow.h"
#include "Colour.h"
#include "CanvasTriangle.h"
#include "Interpolation.h"
#include "Constants.h"

struct BarycentricCoordinates {
	float A;
	float B;
	float C;
};

namespace Triangle {
	void sortTriangle(std::array<CanvasPoint, 3>& vertices);

	void drawLine(DrawingWindow& window, CanvasPoint start, CanvasPoint end, Colour color);

	void drawStrokedTriangle(DrawingWindow& window, CanvasTriangle triangle, Colour color);

	void drawRasterizedTriangle(DrawingWindow& window, CanvasTriangle triangle, Colour color);

	uint32_t getTexture(BarycentricCoordinates coordinates, std::array<CanvasPoint, 3> sortedVertices, TextureMap textures);

	void drawRasterizedTriangle(DrawingWindow& window, CanvasTriangle triangle, TextureMap textures);

	void drawRandomTriangle(DrawingWindow& window, bool isFilled);

	BarycentricCoordinates barycentric(const std::array<CanvasPoint, 3>& sortedVertices, glm::vec2 encodedVertex);

}