#include "Lighting.h"

LightOptions::LightOptions(bool initAmb, bool initShadow, bool initProx, bool initInc, bool initSpec) :
	useShadow(initShadow), useProximity(initProx), useIncidence(initInc), useSpecular(initSpec), useAmbience(initAmb) {}