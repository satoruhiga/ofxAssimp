#pragma once

#include "Constants.h"

#include "Skin.h"

OFX_ASSIMP_BEGIN_NAMESPACE

class Scene;
class Skin;
class Material;

class Mesh {
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
	string name;
	
	Material* material;

	ofVboMesh mesh;
	void setupMesh(Scene* scene, aiMesh* assimp_mesh);
	
	Skin::Ref skin;
	void setupSkin(Scene* scene, aiMesh* assimp_mesh);
};

OFX_ASSIMP_END_NAMESPACE