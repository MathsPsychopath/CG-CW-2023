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
	void operator+=(float highlight);

	uint32_t asNumeric();
	void applyAmbience(float ambientLight, Colour original);
};

std::ostream &operator<<(std::ostream &os, const Colour &colour);
