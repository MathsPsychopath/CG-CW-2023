#include "Lighting.h"

Lighting lighting(true, false, true, true, false, true);

Lighting::Lighting(bool initAmb, bool initShadow, bool initDiffuse, bool initSpec, bool initPhong, bool initSoft) :
	useShadow(initShadow), useProximity(initDiffuse), useIncidence(initDiffuse), useSpecular(initSpec),
	useAmbience(initAmb), usePhong(initPhong), useSoftShadow(initSoft) {}

glm::vec3 Lighting::sampleLightPosition(const glm::vec3 lightPosition, float lightRadius) {
	std::normal_distribution<float> distribution(0.0f, lightRadius);
	std::default_random_engine numberGen(std::chrono::system_clock::now().time_since_epoch().count());

	glm::vec3 offset(distribution(numberGen), distribution(numberGen), distribution(numberGen));
	return lightPosition + offset;
}

