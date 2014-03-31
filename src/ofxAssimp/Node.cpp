#include "Node.h"

#include "Scene.h"
#include "Mesh.h"

OFX_ASSIMP_BEGIN_NAMESPACE

Node::Node(Scene* scene, aiNode* node)
: scene(scene)
, node(node)
, anim(NULL)
, ofNode() {
	name = node->mName.data;
	setTransformMatrix(toOF(node->mTransformation));
}

void Node::draw() {
	transformGL();
	
	for (int i = 0; i < meshes.size(); i++) {
		if (meshes[i].lock())
		{
			// meshes[i]->draw();
		}
	}
	
	restoreTransformGL();
}

void Node::debugDraw() {
	transformGL();
	{
		for (int i = 0; i < meshes.size(); i++) {
			if (meshes[i].lock())
			{
				// (*meshes[i])->draw();
			}
		}
		
		ofDrawAxis(0.5);
		ofPushStyle();
		{
			ofSetColor(255);
			ofNoFill();
			ofDrawBox(0.5);
		}
		ofPopStyle();
	}
	restoreTransformGL();
}

void Node::setupMeshLink() {
	for (int i = 0; i < node->mNumMeshes; i++) {
		unsigned int idx = node->mMeshes[i];
		meshes.push_back(scene->getMesh(idx));
	}
}

void Node::setupInitialTransform() {
	initialTransform = getGlobalTransformMatrix();
	initialTransformInv = initialTransform.getInverse();
}

OFX_ASSIMP_END_NAMESPACE