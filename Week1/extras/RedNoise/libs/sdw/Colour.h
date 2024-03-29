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
	Colour(uint32_t argb);

	void operator*=(float colorScale);
	Colour operator*(float colorScale);
	void operator+=(float highlight);
	Colour operator+(Colour other);
	Colour operator-(Colour other);

	uint32_t asNumeric();
};

std::ostream &operator<<(std::ostream &os, const Colour &colour);
