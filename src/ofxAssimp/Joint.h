#pragma once

#include "Constants.h"

OFX_ASSIMP_BEGIN_NAMESPACE

class Scene;
class Node;

class Joint {
	friend class Mesh;
	
public:
	
	typedef ofPtr<Joint> Ref;
	
	Joint(Scene* scene, Mesh* mesh, aiBone* bone);
	
	void updateJointColor(vector<ofFloatColor>& colors);
	void updateJointTransform(const vector<ofVec3f>& verts_in,
							  vector<ofVec3f>& verts_out,
							  const vector<ofVec3f>& norms_in,
							  vector<ofVec3f>& norms_out);
	
	aiBone* get() { return bone; }
	const aiBone* get() const { return bone; }
	
protected:
	aiBone* bone;
	
	Scene* scene;
	Mesh* mesh;
	Node* node;
	
	string name;
	
	ofFloatColor skinColor;
	
	void setupGlobalJointPosition();
};

OFX_ASSIMP_END_NAMESPACE