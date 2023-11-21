#include "FileReader.h"

FileReader::FileReader() {
	this->supportedColors = std::unordered_map<std::string, Colour>();
}

PolygonData FileReader::readOBJFile(std::string filename, float scaleFactor, glm::vec2 textureScales) {
	if (this->supportedColors.empty()) {
		std::cout << "No palette detected" << std::endl;
		return PolygonData();
	}
	
	std::ifstream inputStream(filename);

	if (!inputStream.is_open()) {
		std::cout << "Could not open file" << std::endl;
		return PolygonData();
	}

	std::string line;
	Colour currentColor;
	int currentVertex = 0;
	std::unordered_map<int, std::set<int>> vertexToTriangles;
	std::vector<ModelTriangle> loadedTriangles;
	std::vector<GouraudVertex> loadedVertices;
	std::vector<TexturePoint> loadedTextures;

	while (std::getline(inputStream, line)) {
		std::vector<std::string> tokens = split(line, ' ');
		// process each line
		std::string identifier = tokens[0];
		if (identifier == "v") {
			glm::vec3 position = {
				std::stof(tokens[1]) * scaleFactor,
				std::stof(tokens[2]) * scaleFactor,
				std::stof(tokens[3]) * scaleFactor
			};
			GouraudVertex vertex(position, currentColor);
			loadedVertices.push_back(vertex);
			vertexToTriangles[currentVertex++] = {};
		}
		else if (identifier == "vt") {
			loadedTextures.push_back(TexturePoint(std::stof(tokens[1]) * textureScales.x, (std::stof(tokens[2])) * textureScales.y));
		}
		else if (identifier == "f") {
			std::vector<std::string> v1 = split(tokens[1], '/');
			std::vector<std::string> v2 = split(tokens[2], '/');
			std::vector<std::string> v3 = split(tokens[3], '/');
			// get the vertex indices required. This changes the vertices to be 0 indexed
			int vIndex1 = std::stoi(v1[0]) - 1;
			int vIndex2 = std::stoi(v2[0]) - 1;
			int vIndex3 = std::stoi(v3[0]) - 1;
			ModelTriangle parsed;
			if (v1[1] == "") {
				parsed = ModelTriangle(vIndex1, vIndex2, vIndex3, currentColor);
				parsed.texturePoints = { -1,-1,-1 };
			}
			else {
				int tIndex1 = std::stoi(v1[1]) - 1;
				int tIndex2 = std::stoi(v2[1]) - 1;
				int tIndex3 = std::stoi(v3[1]) - 1;
				parsed = ModelTriangle(vIndex1, vIndex2, vIndex3, tIndex1, tIndex2, tIndex3);
				parsed.colour = Colour();
			}
			loadedTriangles.push_back(parsed);
			vertexToTriangles[vIndex1].insert(loadedTriangles.size() - 1);
			vertexToTriangles[vIndex2].insert(loadedTriangles.size() - 1);
			vertexToTriangles[vIndex3].insert(loadedTriangles.size() - 1);
		}
		else if (identifier == "usemtl") {
			currentColor = supportedColors[tokens[1]];
		}
	}

	inputStream.close();
	PolygonData output = { vertexToTriangles, loadedTriangles, loadedVertices, loadedTextures };
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

