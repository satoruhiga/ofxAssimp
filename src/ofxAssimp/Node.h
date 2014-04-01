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

	aiNode* get() { return node; }
	const aiNode* get() const { return node; }
	
	const ofMatrix4x4& getMatrix() const { return matrix; }
	const ofMatrix4x4& getGlobalMatrix() const { return global_matrix_cache; }
	const ofMatrix4x4& getGlobalRigidTransformMatrix() const { return global_rigid_transform; }

private:
	string name;

	Scene* scene;
	aiNode* node;
	
	Node *parent;

	double tick_par_second;
	map<double, ofMatrix4x4> animation;

	ofMatrix4x4 initialTransform, initialTransformInv;

	void setupNodeAnimation(aiNodeAnim* anim, double tick_par_second);
	
	ofMatrix4x4 matrix, global_matrix_cache;
	ofMatrix4x4 global_rigid_transform;
	void updateGlobalMatrixCache();
	
	vector<Mesh*> meshes;
	void addMeshReference(Mesh *mesh);
};

OFX_ASSIMP_END_NAMESPACE