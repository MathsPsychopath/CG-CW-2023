#include "Lighting.h"

LightOptions lighting(true, true, false, false, true, false);

LightOptions::LightOptions(bool initAmb, bool initShadow, bool initProx, bool initInc, bool initSpec, bool initPhong) :
	useShadow(initShadow), useProximity(initProx), useIncidence(initInc), useSpecular(initSpec), useAmbience(initAmb), usePhong(initPhong) {}