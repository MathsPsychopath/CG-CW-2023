#include "FileReader.h"

FileReader::FileReader() {
	this->supportedColors = std::unordered_map<std::string, Colour>();
	this->loadedVertices = std::unordered_map<std::string, glm::vec3>();
}

std::vector<ModelTriangle> FileReader::readOBJFile(std::string filename, float scaleFactor) {
	if (this->supportedColors.empty()) {
		std::cout << "No palette detected" << std::endl;
		return {};
	}
	
	std::ifstream inputStream(filename);

	if (!inputStream.is_open()) {
		std::cout << "Could not open file" << std::endl;
		return {};
	}

	std::string line;
	std::vector<ModelTriangle> output = {};
	int currentVertex = 1;
	Colour currentColor;

	while (std::getline(inputStream, line)) {
		std::vector<std::string> tokens = split(line, ' ');
		// process each line
		std::string identifier = tokens[0];
		if (identifier == "v") {
			glm::vec3 vertex = {
				std::stof(tokens[1]) * scaleFactor,
				std::stof(tokens[2]) * scaleFactor,
				std::stof(tokens[3]) * scaleFactor
			};
			this->loadedVertices[std::to_string(currentVertex) + "/"] = vertex;
			currentVertex++;
		}
		else if (identifier == "f") {
			ModelTriangle parsed(
				this->loadedVertices[tokens[1]], 
				this->loadedVertices[tokens[2]], 
				this->loadedVertices[tokens[3]], 
				currentColor
			);
			std::cout << parsed << std::endl;
			output.push_back(parsed);
		}
		else if (identifier == "usemtl") {
			currentColor = supportedColors[tokens[1]];
		}
	}

	inputStream.close();
	return output;
}

void FileReader::readMTLFile(std::string filename) {
	std::ifstream inputStream(filename);

	if (!inputStream.is_open()) {
		std::cout << "Could not open file" << std::endl;
		return;
	}

	std::string line;
	std::string currentColor;

	while (std::getline(inputStream, line)) {
		std::vector<std::string> tokens = split(line, ' ');
		std::string identifier = tokens[0];

		if (identifier == "newmtl") {
			currentColor = tokens[1];
		}
		else if (identifier == "Kd") {
			this->supportedColors[currentColor] = Colour(
				std::round(std::stof(tokens[1]) * 255),
				std::round(std::stof(tokens[2]) * 255),
				std::round(std::stof(tokens[3]) * 255)
			);
		}
	}
	inputStream.close();
}

