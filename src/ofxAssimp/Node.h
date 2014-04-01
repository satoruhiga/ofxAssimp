#pragma once

#include "Constants.h"
#include "Mesh.h"

OFX_ASSIMP_BEGIN_NAMESPACE

class Scene;

class Node {
	friend class Scene;

public:
	
	typedef ofPtr<Node> Ref;
	
	Node(Scene* scene, aiNode* node, Node* parent);
	
	void update(float sec);
	
	void draw();
	void debugDraw();
	
	const string& getName() const { return name; }

	inline const ofMatrix4x4& getInitialTransform() const {
		return initialTransform;
	}

	inline const ofMatrix4x4& getInitialTransformInv() const {
		return initialTransformInv;
	}

//	OF_DEPRECATED(ofMatrix4x4 getBoneMatrix() const {
//		return initialTransformInv * getGlobalTransformMatrix();
//	});

	aiNode* get() { return node; }
	const aiNode* get() const { return node; }

	inline void pushMatrix() const {
		ofPushMatrix();
		ofMultMatrix(global_matrix_cache);
	}
	
	inline void popMatrix() const {
		ofPopMatrix();
	}
	
private:
	string name;

	Scene* scene;
	aiNode* node;
	
	Node *parent;

	vector<Mesh::WeakRef> meshes;
	
	double tick_par_second;
	map<double, ofMatrix4x4> animation;

	ofMatrix4x4 initialTransform, initialTransformInv;

	void setupMeshLink();
	void setupNodeAnimation(aiNodeAnim* anim, double tick_par_second);
	
	ofMatrix4x4 matrix, global_matrix_cache;
	void updateGlobalMatrixCache();
};

OFX_ASSIMP_END_NAMESPACE