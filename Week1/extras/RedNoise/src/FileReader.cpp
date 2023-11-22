#include "FileReader.h"

FileReader::FileReader() {
	this->supportedColors = std::unordered_map<std::string, Colour>();
}

void FileReader::addFileData(PolygonData& objects, std::ifstream& valid_filestream, float scaleFactor, glm::vec2 textureScales, int offset) {
	std::string line;
	Colour currentColor;
	int currentVertex = 0;
	glm::vec3 translation = { 0,0,0 };
	while (std::getline(valid_filestream, line)) {
		std::vector<std::string> tokens = split(line, ' ');
		// process each line
		std::string identifier = tokens[0];
		if (identifier == "v") {
			glm::vec3 position = {
				std::stof(tokens[1]) * scaleFactor + translation.x,
				std::stof(tokens[2]) * scaleFactor + translation.y,
				std::stof(tokens[3]) * scaleFactor + translation.z
			};
			GouraudVertex vertex(position, currentColor);
			objects.loadedVertices.push_back(vertex);
			objects.vertexToTriangles[currentVertex++] = {};
		}
		else if (identifier == "vt") {
			objects.loadedTextures.push_back(TexturePoint(std::stof(tokens[1]) * textureScales.x, (std::stof(tokens[2])) * textureScales.y));
		}
		else if (identifier == "f") {
			std::vector<std::string> v1 = split(tokens[1], '/');
			std::vector<std::string> v2 = split(tokens[2], '/');
			std::vector<std::string> v3 = split(tokens[3], '/');
			// get the vertex indices required. This changes the vertices to be 0 indexed
			int vIndex1 = std::stoi(v1[0]) + offset - 1;
			int vIndex2 = std::stoi(v2[0]) + offset - 1;
			int vIndex3 = std::stoi(v3[0]) + offset - 1;
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
			objects.loadedTriangles.push_back(parsed);
			objects.vertexToTriangles[vIndex1].insert(objects.loadedTriangles.size() - 1);
			objects.vertexToTriangles[vIndex2].insert(objects.loadedTriangles.size() - 1);
			objects.vertexToTriangles[vIndex3].insert(objects.loadedTriangles.size() - 1);
		}
		else if (identifier == "usemtl") {
			currentColor = supportedColors[tokens[1]];
		}
		else if (identifier == "o" && tokens[1] == "red_sphere") {
			translation = { -0.5, -1.2, 0.3 };
		}
	}
}

void FileReader::appendPolygonData(PolygonData& objects, std::string filename) {
	if (this->supportedColors.empty()) {
		std::cout << "No palette detected" << std::endl;
		return;
	}
	
	std::ifstream inputStream(filename);
	
	if (!inputStream.is_open()) {
		std::cout << "Could not open file" << std::endl;
		return;
	}
	this->addFileData(objects, inputStream, 0.35, glm::vec2{ 0,0 }, objects.loadedVertices.size());
	inputStream.close();
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
	int currentVertex = 0;
	std::unordered_map<int, std::set<int>> vertexToTriangles;
	std::vector<ModelTriangle> loadedTriangles;
	std::vector<GouraudVertex> loadedVertices;
	std::vector<TexturePoint> loadedTextures;
	PolygonData output = { vertexToTriangles, loadedTriangles, loadedVertices, loadedTextures };
	this->addFileData(output, inputStream, scaleFactor, textureScales);
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

