#include "Scene.h"

#include "Mesh.h"
#include "Node.h"

OFX_ASSIMP_BEGIN_NAMESPACE

Scene::Scene()
	: scene(NULL)
    , duration(0)
    , play_head(0)
    {}
Scene::~Scene() { unload(); }

bool Scene::load(string path, bool optimize, Handedness handness) {
	file.open(path);
	return load(file.readToBuffer(), optimize, handness, file.getExtension().c_str());
}

bool Scene::load(const ofBuffer& buffer, bool optimize, Handedness handness, const char* extension) {
	unload();

	unsigned int flags = aiProcessPreset_TargetRealtime_MaxQuality;

	if (optimize) {
		flags |= aiProcess_ImproveCacheLocality | 
				 aiProcess_JoinIdenticalVertices |
				 aiProcess_RemoveRedundantMaterials;
	}

	if (handness == LEFT_HANDED) {
		flags |= aiProcess_ConvertToLeftHanded;
	}

	scene = aiImportFileFromMemory(buffer.getBinaryBuffer(), buffer.size(),
								   flags, extension);
	string err_str = aiGetErrorString();
	if (!err_str.empty())
	{
		ofLogError("ofxAssimp::Scene::load") << err_str;
	}
	assert(scene);
	
	setupResources();
	setupNodes();
	setupAnimations();

	return true;
}

void Scene::unload() {
	nodes.clear();
	nodeNames.clear();
	resource.reset();
    root_node.reset();
    
    duration = 0;
    play_head = 0;
    
    file = ofFile();
	
	if (scene) {
		aiReleaseImport(scene);
		scene = NULL;
	}
}

void Scene::dumpScene() {
	cout << "==== Node ====" << endl;
	for (int i = 0; i < nodeNames.size(); i++) {
		printf("%03i: %s\n", i, nodeNames[i].c_str());
	}
}

void Scene::update() {
	update(fmodf(ofGetElapsedTimef(), duration));
}

void Scene::update(float sec) {
	if (play_head == sec) return;
	play_head = sec;
	
	if (root_node) {
		root_node->updateNodeAnimation(sec);
		root_node->update();
	}
}

void Scene::draw() {
	map<string, Node::Ref>::iterator it = nodes.begin();
	while (it != nodes.end()) {
		it->second->draw();
		it++;
	}
}

void Scene::debugDraw() {
	map<string, Node::Ref>::iterator it = nodes.begin();
	while (it != nodes.end()) {
		it->second->debugDraw();
		it++;
	}
}

Node::Ref Scene::nodeSetupVisiter(Scene* s, aiNode* node, Node* parent) {
	Node::Ref n = Node::Ref(new Node(s, node, parent));
	s->nodes[n->getName()] = n;
	s->nodeNames.push_back(n->getName());

	for (int i = 0; i < node->mNumChildren; i++) {
		aiNode* o = node->mChildren[i];
		nodeSetupVisiter(s, o, n.get());
	}

	return n;
}

void Scene::setupNodes() {
	nodes.clear();
	nodeNames.clear();

	root_node = nodeSetupVisiter(this, scene->mRootNode, NULL);
}

void Scene::setupAnimations() {
	duration = 0;
	play_head = 0;
	
	for (int i = 0; i < scene->mNumAnimations; i++) {
		aiAnimation* anim = scene->mAnimations[i];
		
		duration = anim->mDuration / anim->mTicksPerSecond;
		
		for (int n = 0; n < anim->mNumChannels; n++) {
			aiNodeAnim* node_anim = anim->mChannels[n];

			if (nodes.find(node_anim->mNodeName.data) != nodes.end()) {
				Node::Ref node = nodes[node_anim->mNodeName.data];
				node->setupNodeAnimation(node_anim, anim->mTicksPerSecond);
			}
		}

		// TODO: mesh anim
		
		break; // use 1st animation for now
	}
}

void Scene::setupResources() {
	resource = Resource::Ref(new Resource(this, file));
}

OFX_ASSIMP_END_NAMESPACE