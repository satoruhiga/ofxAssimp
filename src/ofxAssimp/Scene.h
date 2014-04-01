#pragma once

#include "Constants.h"

#include "Resource.h"
#include "Node.h"
#include "Mesh.h"

OFX_ASSIMP_BEGIN_NAMESPACE

class Scene {
	friend class Node;
	friend class Joint;
	friend class Mesh;

public:
	Scene();
	~Scene();

	bool load(string path, bool optimize = false);
	bool load(const ofBuffer& buffer, bool optimize = false,
			  const char* extension = "");
	void unload();

	void dumpScene();

	void update();
	void update(float sec);

	void draw();
	void debugDraw();

	inline const aiScene* get() const { return scene; }

	size_t getNumMesh() const { return meshes.size(); }
	Mesh::Ref getMesh(size_t index) { return meshes.at(index); }

	inline const vector<string>& getNodeNames() const { return nodeNames; }
	Node::Ref getNodeByName(const string& name){
		if (nodes.find(name) == nodes.end()) return Node::Ref();
		return nodes[name];
	}

protected:
	const aiScene* scene;

	ofFile file;

	vector<Mesh::Ref> meshes;

	vector<string> nodeNames;
	map<string, Node::Ref> nodes;

	Resource::Ref resource;
	
	double duration;

	static Node::Ref nodeSetupVisiter(Scene* s, aiNode* node, Node* parent);

	void setupResources();
	void setupNodes();
	void setupMeshes();
	void setupAnimations();
};

OFX_ASSIMP_END_NAMESPACE
