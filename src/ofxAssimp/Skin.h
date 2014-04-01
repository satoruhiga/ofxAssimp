#pragma once

#include "Constants.h"

OFX_ASSIMP_BEGIN_NAMESPACE

class Scene;
class Mesh;

class Skin {
public:
	
	Skin(Scene* scene, Mesh* mesh);

	void update();
	
	void debugDraw();
	
};

OFX_ASSIMP_END_NAMESPACE
