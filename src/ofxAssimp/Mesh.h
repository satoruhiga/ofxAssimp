#pragma once

#include "Constants.h"

OFX_ASSIMP_BEGIN_NAMESPACE

class Scene;

class Mesh {
	friend class Joint;

public:
	
	typedef ofPtr<Mesh> Ref;
	typedef std::weak_ptr<Mesh> WeakRef;
	
	Mesh(Scene* scene, aiMesh* assimp_mesh);
	~Mesh();
	
	void update();
	
	void draw(ofPolyRenderMode renderType = OF_MESH_FILL);
	void debugDraw();

	const string& getName() { return name; }

	ofMesh getMesh() { return mesh; }
	const ofMesh& getMesh() const { return mesh; }

protected:
	Scene* scene;
	aiMesh* assimp_mesh;

	string name;

	ofMesh originalMesh;
	ofMesh mesh;

	map<string, Joint*> joints;

	void setupMesh();
	void setupSkeleton();
};

OFX_ASSIMP_END_NAMESPACE