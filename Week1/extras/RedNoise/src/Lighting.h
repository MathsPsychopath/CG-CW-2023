#pragma once

struct LightOptions {
	bool useShadow;
	bool useProximity;
	bool useIncidence;
	bool useSpecular;
	bool useAmbience;

	LightOptions(bool initAmb, bool initShadow, bool initProx, bool initInc, bool initSpec);
};