#pragma once

#include "Constants.h"

OFX_ASSIMP_BEGIN_NAMESPACE

class Material {
public:
	Material()
		: hasGlMaterial(false)
		, twoSided(true)
		, blendMode(OF_BLENDMODE_ALPHA) {}

	bool hasGlMaterial;
	ofMaterial material;
	ofBlendMode blendMode;

	ofTexture diffuseTex;

	bool twoSided;

	void begin(ofPolyRenderMode renderType) {
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
		glEnable(GL_NORMALIZE);

		ofPushStyle();

		if (hasGlMaterial) material.begin();

		if (diffuseTex.isAllocated()) diffuseTex.bind();

		ofEnableBlendMode(blendMode);

		if (twoSided)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);

		glPolygonMode(GL_FRONT_AND_BACK, ofGetGLPolyMode(renderType));
	}

	void end() {
		if (diffuseTex.isAllocated()) diffuseTex.unbind();

		if (hasGlMaterial) material.end();

		ofPopStyle();

		glPopClientAttrib();
		glPopAttrib();
	}
};

class Resource {
public:
	
	typedef ofPtr<Resource> Ref;
	
	vector<Material> materials;
};

OFX_ASSIMP_END_NAMESPACE