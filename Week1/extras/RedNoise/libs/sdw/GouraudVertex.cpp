#include "GouraudVertex.h"

GouraudVertex::operator glm::vec3() {
	return this->position;
}

GouraudVertex::GouraudVertex() : ambient(0, 0, 0), proximity(0), incidental(0), specular(0), diffuse(0) {};

GouraudVertex::GouraudVertex(glm::vec3 position, Colour color) : ambient(0,0,0), proximity(0), incidental(0), specular(0), diffuse(0), position(position), originalColor(color) {};

std::ostream& operator<<(std::ostream& os, const GouraudVertex& gv) {
	os << "{ ambient: " << gv.ambient << ", proximity: " << gv.proximity << ", incidental: " << gv.incidental << ", specular: " << gv.specular << " }\n";
	return os;
}
