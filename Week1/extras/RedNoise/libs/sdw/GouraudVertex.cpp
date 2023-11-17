#include "GouraudVertex.h"

GouraudVertex::operator glm::vec3() {
	return this->position;
}

GouraudVertex::GouraudVertex() : ambient(0), proximity(0), incidental(0), specular(0) {};

GouraudVertex::GouraudVertex(glm::vec3 position) : ambient(0), proximity(0), incidental(0), specular(0), position(position) {};
