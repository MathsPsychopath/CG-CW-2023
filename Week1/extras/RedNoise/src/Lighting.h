#pragma once
#include <random>
#include <chrono>
#include <glm/glm.hpp>

struct Lighting{
	bool useShadow;
	bool useProximity;
	bool useIncidence;
	bool useSpecular;
	bool useAmbience;
	bool usePhong;
	bool useSoftShadow;
	bool useReflections;
	bool useFilter;

	Lighting(bool initAmb, bool initShadow, bool initDiffuse, bool initSpec, bool initPhong, bool initSoft);

	static glm::vec3 sampleLightPosition(const glm::vec3 lightPosition, float lightRadius);
};

extern Lighting lighting;