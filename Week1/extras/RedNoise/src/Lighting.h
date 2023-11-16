#pragma once

struct LightOptions {
	bool useShadow;
	bool useProximity;
	bool useIncidence;
	bool useSpecular;

	LightOptions(bool initShadow, bool initProx, bool initInc, bool initSpec);
};