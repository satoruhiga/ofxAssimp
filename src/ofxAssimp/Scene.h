#pragma once

#include "Constants.h"

#include "Resource.h"
#include "Node.h"
#include "Mesh.h"

OFX_ASSIMP_BEGIN_NAMESPACE

class Scene {
	friend class Node;
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

	inline const vector<string>& getNodeNames() const { return nodeNames; }
	Node::Ref getNodeByName(const string& name){
		if (nodes.find(name) == nodes.end()) return Node::Ref();
		return nodes[name];
	}

protected:
	const aiScene* scene;

	ofFile file;

	vector<string> nodeNames;
	map<string, Node::Ref> nodes;
	Node::Ref root_node;

	Resource::Ref resource;
	
	double duration;

	static Node::Ref nodeSetupVisiter(Scene* s, aiNode* node, Node* parent);

	void setupResources();
	void setupNodes();
	void setupAnimations();
};

OFX_ASSIMP_END_NAMESPACE
