#include "Scene.h"

#include "Mesh.h"
#include "Node.h"

OFX_ASSIMP_BEGIN_NAMESPACE

Scene::Scene()
	: scene(NULL) {}
Scene::~Scene() { unload(); }

bool Scene::load(string path, bool optimize) {
	file.open(path);
	return load(file.readToBuffer(), optimize, file.getExtension().c_str());
}

bool Scene::load(const ofBuffer& buffer, bool optimize, const char* extension) {
	unload();

	unsigned int flags = aiProcessPreset_TargetRealtime_MaxQuality |
						 aiProcess_Triangulate;

	if (optimize) {
		flags |= aiProcess_ImproveCacheLocality | aiProcess_OptimizeGraph |
				 aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices |
				 aiProcess_RemoveRedundantMaterials;
	}

	if (ofGetCoordHandedness() == OF_LEFT_HANDED) {
		flags |= aiProcess_ConvertToLeftHanded;
	}

	scene = aiImportFileFromMemory(buffer.getBinaryBuffer(), buffer.size(),
								   flags, extension);
	assert(scene);
	
	setupResources();
	setupNodes();
	setupMeshes();
	setupAnimations();

	return true;
}

void Scene::unload() {
	nodes.clear();
	nodeNames.clear();
	meshes.clear();
	resource.reset();
	
	if (scene) {
		aiReleaseImport(scene);
		scene = NULL;
	}
}

void Scene::dumpScene() {
	cout << "==== Mesh ====" << endl;
	for (int i = 0; i < meshes.size(); i++) {
		printf("%03i: %s\n", i, meshes[i]->getName().c_str());
	}

	cout << endl;

	cout << "==== Node ====" << endl;
	for (int i = 0; i < nodeNames.size(); i++) {
		printf("%03i: %s\n", i, nodeNames[i].c_str());
	}
}

void Scene::update() {
	update(fmodf(ofGetElapsedTimef(), duration));
}

void Scene::update(float sec) {
	if (root_node) {
		root_node->updateNodeAnimationRecursive(root_node.get(), sec);
	}
	
	for (int i = 0; i < meshes.size(); i++) {
		meshes[i]->update();
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

void Scene::setupMeshes() {
	meshes.clear();

	for (int i = 0; i < scene->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[i];
		Mesh::Ref o = Mesh::Ref(new Mesh(this, mesh));
		meshes.push_back(o);
	}

	map<string, Node::Ref>::iterator it = nodes.begin();
	while (it != nodes.end()) {
		Node::Ref& o = it->second;
		aiNode *n = o->get();
		
		for (int i = 0; i < n->mNumMeshes; i++) {
			unsigned int id = n->mMeshes[i];
			o->addMeshReference(meshes[id].get());
		}
		
		it++;
	}
}
void Scene::setupAnimations() {
	duration = 0;
	
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
	resource = Resource::Ref(new Resource);
	
	for (int i = 0; i < scene->mNumMaterials; i++) {
		aiMaterial* mtl = scene->mMaterials[i];
		resource->materials.push_back(Material());
		
		Material& m = resource->materials.back();
		
		aiColor4D dcolor, scolor, acolor, ecolor;
		
		if (AI_SUCCESS ==
			aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &dcolor)) {
			m.material.setDiffuseColor(toOF(dcolor));
			m.hasGlMaterial = true;
		}
		
		if (AI_SUCCESS ==
			aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &scolor)) {
			m.material.setSpecularColor(toOF(scolor));
			m.hasGlMaterial = true;
		}
		
		if (AI_SUCCESS ==
			aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &acolor)) {
			m.material.setAmbientColor(toOF(acolor));
			m.hasGlMaterial = true;
		}
		
		if (AI_SUCCESS ==
			aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &ecolor)) {
			m.material.setEmissiveColor(toOF(ecolor));
			m.hasGlMaterial = true;
		}
		
		float shininess;
		if (AI_SUCCESS ==
			aiGetMaterialFloat(mtl, AI_MATKEY_SHININESS, &shininess)) {
			m.material.setShininess(shininess);
			m.hasGlMaterial = true;
		}
		
		int blendMode;
		if (AI_SUCCESS ==
			aiGetMaterialInteger(mtl, AI_MATKEY_BLEND_FUNC, &blendMode)) {
			if (blendMode == aiBlendMode_Default) {
				m.blendMode = OF_BLENDMODE_ALPHA;
			} else {
				m.blendMode = OF_BLENDMODE_ADD;
			}
		}
		
		unsigned int max = 1;
		int two_sided;
		if ((AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_TWOSIDED,
													 &two_sided, &max)) &&
			two_sided)
			m.twoSided = true;
		else
			m.twoSided = false;
		
		int texIndex = 0;
		aiString texPath;
		
		if (AI_SUCCESS ==
			mtl->GetTexture(aiTextureType_DIFFUSE, texIndex, &texPath)) {
			string modelFolder = file.getEnclosingDirectory();
			string relTexPath =
			ofFilePath::getEnclosingDirectory(texPath.data, false);
			string texFile = ofFilePath::getFileName(texPath.data);
			string realPath = modelFolder + relTexPath + texFile;
			
			if (!ofFile::doesFileExist(realPath) ||
				!ofLoadImage(m.diffuseTex, realPath)) {
				ofLog(OF_LOG_ERROR, string("error loading image ") +
					  file.getFileName() + " " + realPath);
			}
		}
	}
}

OFX_ASSIMP_END_NAMESPACE