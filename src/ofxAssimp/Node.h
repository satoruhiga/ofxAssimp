#pragma once

#include "Constants.h"
#include "Mesh.h"

OFX_ASSIMP_BEGIN_NAMESPACE

class Scene;

class Node : public ofNode {
	friend class Scene;

public:
	
	typedef ofPtr<Node> Ref;
	
	Node(Scene* scene, aiNode* node);
	
	void draw();
	void debugDraw();
	
	const string& getName() const { return name; }

	inline const ofMatrix4x4& getInitialTransform() const {
		return initialTransform;
	}

	inline const ofMatrix4x4& getInitialTransformInv() const {
		return initialTransformInv;
	}

	OF_DEPRECATED(ofMatrix4x4 getBoneMatrix() const {
		return initialTransformInv * getGlobalTransformMatrix();
	});

	aiNode* get() { return node; }
	const aiNode* get() const { return node; }

private:
	string name;

	Scene* scene;
	aiNode* node;
	aiNodeAnim* anim;

	vector<Mesh::WeakRef> meshes;

	ofMatrix4x4 initialTransform, initialTransformInv;

	void setupMeshLink();
	void setupInitialTransform();
};

OFX_ASSIMP_END_NAMESPACE