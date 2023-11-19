#pragma once
#include <ModelTriangle.h>
#include <string>
#include <fstream>
#include <Utils.h>
#include <PolygonData.h>
#include <sstream>

class FileReader {
public:
	std::unordered_map<std::string, Colour> supportedColors;

	FileReader();

	PolygonData readOBJFile(std::string filename, float scaleFactor, glm::vec2 textureScales);

	void readMTLFile(std::string filename);
};

