#pragma once
#include <ModelTriangle.h>
#include <string>
#include <fstream>
#include <Utils.h>
#include <PolygonData.h>
#include <sstream>

class FileReader {
private:
	void addFileData(PolygonData& objects, std::ifstream& valid_filestream, float scaleFactor, glm::vec2 textureScales, int offset = 0);

public:
	std::unordered_map<std::string, Colour> supportedColors;

	FileReader();

	PolygonData readOBJFile(std::string filename, float scaleFactor, glm::vec2 textureScales = glm::vec2{1,1});

	void appendPolygonData(PolygonData& objects, std::string filename);

	void readMTLFile(std::string filename);
};

