#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <array>
#include "TextureMap.h"

struct InterpolatedTriangle {
	std::vector<float> topLeft;
	std::vector<float> topRight;
	std::vector<float> leftBottom;
	std::vector<float> rightBottom;
};

class Interpolate {
private:
	static std::array<glm::vec2, 4> projectLeftVertex(const std::array<glm::vec2, 3>& sortedVertices);
public:
	static std::vector<float> singleFloat(float from, float to, int numberOfValues);
	static std::vector<glm::vec3> threeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValues);

	/// <summary>
	/// This takes the vertices in sorted order by Y-value, and does interpolation
	/// </summary>
	/// <param name="sortedVertices">vertices sorted by y-value lowest to highest</param>
	/// <returns>Struct pointing to interpolated values</returns>
	static InterpolatedTriangle triangle(const std::array<glm::vec2, 3>& sortedVertices);

	/// <summary>
	/// This takes the vertices in sorted order by Y-value, the ratio of y diff / y length, and interpolates 
	/// the texture colors in the line
	/// </summary>
	/// <param name="interpolations">the X values of the edges from interpolating</param>
	/// <param name="ratio">how far the scanline is down the triangle</param>
	/// <param name="isBottomTriangle">which triangle are we rendering?</param>
	/// <param name="textures">The texture map to source from</param>
	/// <returns>List of colors representing the texture</returns>
	static std::vector<uint32_t> triangleTexture(std::array<glm::vec2,3> textureVertices, float ratio, int pixels, bool isBottomTriangle, TextureMap textures);
};