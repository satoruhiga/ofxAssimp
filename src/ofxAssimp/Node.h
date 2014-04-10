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
	
	void update();
	
	void draw();
	void debugDraw();
	
	const string& getName() const { return name; }

	aiNode* get() { return node; }
	const aiNode* get() const { return node; }
	
	const ofMatrix4x4& getMatrix() const { return matrix; }
	void setMatrix(const ofMatrix4x4& m) { matrix = m; is_dirty = true; }
	
	const ofMatrix4x4& getGlobalMatrix() const { return global_matrix_cache; }
	const ofMatrix4x4& getGlobalRigidTransformMatrix() const { return global_rigid_transform; }

	Node* getParent() const { return parent; }
	
	inline bool hasAnimation() const { return !animation.empty(); }
	inline bool hasMesh() const { return !meshes.empty(); }
	
private:
	
	aiNode* node;
	
	string name;
	bool is_dirty;
	
	Node *parent;
	vector<Node*> children;

	void setupNodeAnimation(aiNodeAnim* anim, double tick_par_second);

	ofMatrix4x4 matrix, global_matrix_cache, global_rigid_transform;
	void updateGlobalMatrixCache();
	
	vector<Mesh::Ref> meshes;
	void setupMesh(Scene* scene, aiNode* node);
	
	double tick_par_second;
	map<double, ofMatrix4x4> animation;
	void updateNodeAnimation(float sec);
};

OFX_ASSIMP_END_NAMESPACE