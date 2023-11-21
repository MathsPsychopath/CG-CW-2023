#pragma once

struct LightOptions {
	bool useShadow;
	bool useProximity;
	bool useIncidence;
	bool useSpecular;
	bool useAmbience;
	bool usePhong;

	LightOptions(bool initAmb, bool initShadow, bool initProx, bool initInc, bool initSpec, bool initPhong);
};

extern LightOptions lighting;