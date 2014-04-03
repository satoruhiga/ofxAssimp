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
	
	void draw();
	void debugDraw();
	
	const string& getName() const { return name; }

	aiNode* get() { return node; }
	const aiNode* get() const { return node; }
	
	const ofMatrix4x4& getMatrix() const { return matrix; }
	const ofMatrix4x4& getGlobalMatrix() const { return global_matrix_cache; }
	const ofMatrix4x4& getGlobalRigidTransformMatrix() const { return global_rigid_transform; }

	Node* getParent() const { return parent; }
	
private:
	
	aiNode* node;
	
	string name;
	
	Scene* scene;
	
	Node *parent;
	vector<Node*> children;

	void setupNodeAnimation(aiNodeAnim* anim, double tick_par_second);

	ofMatrix4x4 matrix, global_matrix_cache, global_rigid_transform;
	void updateGlobalMatrixCache();
	
	vector<Mesh*> meshes;
	void addMeshReference(Mesh *mesh);
	
	double tick_par_second;
	map<double, ofMatrix4x4> animation;
	void updateNodeAnimation(float sec);
	
	static void updateNodeAnimationRecursive(Node *node, float sec);
};

OFX_ASSIMP_END_NAMESPACE