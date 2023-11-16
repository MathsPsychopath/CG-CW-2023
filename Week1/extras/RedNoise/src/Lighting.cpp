#include "Lighting.h"

LightOptions::LightOptions(bool initShadow, bool initProx, bool initInc, bool initSpec, bool initAmb) :
	useShadow(initShadow), useProximity(initProx), useIncidence(initInc), useSpecular(initSpec), useAmbience(initAmb) {}