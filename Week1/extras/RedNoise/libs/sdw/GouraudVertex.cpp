#include "GouraudVertex.h"

GouraudVertex::operator glm::vec3() {
	return this->position;
}

GouraudVertex::GouraudVertex() = default;
