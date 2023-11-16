#pragma once

#include <iostream>
#include <utility>
#include <glm/gtx/extented_min_max.hpp>

struct Colour {
	std::string name;
	int red{};
	int green{};
	int blue{};
	Colour();
	Colour(int r, int g, int b);
	Colour(std::string n, int r, int g, int b);

	void operator*=(float colorScale);

	uint32_t asNumeric();
};

std::ostream &operator<<(std::ostream &os, const Colour &colour);
