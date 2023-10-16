#pragma once
#include <vector>
#include <ModelTriangle.h>
#include <unordered_map>
#include <string>
#include <fstream>
#include <Utils.h>

class FileReader {
public:
	std::unordered_map<std::string, Colour> supportedColors;
	std::unordered_map<std::string, glm::vec3> loadedVertices;


	std::vector<ModelTriangle> readOBJFile(std::string filename, float scaleFactor);

	void readMTLFile(std::string filename);
};

