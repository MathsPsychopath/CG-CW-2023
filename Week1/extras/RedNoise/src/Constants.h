#pragma once

const int WIDTH = 320;
const int HEIGHT = 240;
const enum RenderType {
	POINTCLOUD,
	WIREFRAME,
	RASTER,
	RAYTRACE,
};
const enum LightType {
	HARD,
	PROXIMITY,
	INCIDENCE,
	DIFFUSE,
	SPECULAR,
};