#pragma once

struct LightOptions {
	bool useShadow;
	bool useProximity;
	bool useIncidence;
	bool useSpecular;
	bool useAmbience;

	LightOptions(bool initShadow, bool initProx, bool initInc, bool initSpec, bool initAmb);
};