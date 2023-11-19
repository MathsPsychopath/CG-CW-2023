#include "Colour.h"

Colour::Colour() = default;
Colour::Colour(int r, int g, int b) : red(r), green(g), blue(b) {}
Colour::Colour(std::string n, int r, int g, int b) :
		name(std::move(n)),
		red(r), green(g), blue(b) {}

std::ostream &operator<<(std::ostream &os, const Colour &colour) {
	os << colour.name << " ["
	   << colour.red << ", "
	   << colour.green << ", "
	   << colour.blue << "]";
	return os;
}

uint32_t Colour::asNumeric() {
	return (255 << 24) + (int(this->red) << 16) + (int(this->green) << 8) + int(this->blue);
}

void Colour::operator*=(float colorScale) {
	this->red = glm::min(int(this->red * colorScale), 255);
	this->blue = glm::min(int(this->blue * colorScale), 255);
	this->green = glm::min(int(this->green * colorScale), 255);
}

Colour Colour::operator*(float colorScale) {
	Colour result;
	result.red = int(this->red * colorScale);
	result.green = int(this->green * colorScale);
	result.blue = int(this->blue * colorScale);
	return result;
}

void Colour::operator+=(float highlight) {
	this->red = glm::min(this->red + int(highlight), 255);
	this->green = glm::min(this->green + int(highlight), 255);
	this->blue = glm::min(this->blue + int(highlight), 255);
}

Colour Colour::operator+(Colour other) {
	Colour result;
	result.red = glm::clamp(this->red + other.red, 0, 255);
	result.green = glm::clamp(this->green + other.green, 0, 255);
	result.blue = glm::clamp(this->blue + other.blue, 0, 255);
	return result;
}
