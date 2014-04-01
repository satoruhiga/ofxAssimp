#include "Node.h"

#include "Scene.h"
#include "Mesh.h"

OFX_ASSIMP_BEGIN_NAMESPACE

Node::Node(Scene* scene, aiNode* node, Node* parent)
	: scene(scene)
	, node(node)
	, parent(parent) {
	name = node->mName.data;

	matrix = toOF(node->mTransformation);
	updateGlobalMatrixCache();

	//	initialTransform = getGlobalTransformMatrix();
	//	initialTransformInv = initialTransform.getInverse();
}

void Node::update(float sec) {
	if (animation.empty()) return;

	map<double, ofMatrix4x4>::iterator it =
		animation.lower_bound(sec * tick_par_second);
	if (it == animation.end()) return;

	matrix = it->second;
	updateGlobalMatrixCache();
}

void Node::draw() {
	pushMatrix();
	
	//	for (int i = 0; i < meshes.size(); i++) {
	//		if (meshes[i].lock())
	//		{
	//			// meshes[i]->draw();
	//		}
	//	}

	popMatrix();
}

void Node::debugDraw() {
	if (parent) {
		parent->pushMatrix();
		ofLine(ofVec3f(0, 0, 0), matrix.getTranslation());
		parent->popMatrix();
	}

	pushMatrix();
	{
		//		for (int i = 0; i < meshes.size(); i++) {
		//			if (meshes[i].lock())
		//			{
		//				// (*meshes[i])->draw();
		//			}
		//		}

		ofDrawAxis(0.5);
		ofPushStyle();
		{
			ofSetColor(255);
			ofNoFill();
			ofDrawBox(0.5);

			ofDrawBitmapString(name, 0, 0);
		}
		ofPopStyle();
	}
	popMatrix();
}

void Node::setupMeshLink() {
	for (int i = 0; i < node->mNumMeshes; i++) {
		unsigned int idx = node->mMeshes[i];
		meshes.push_back(scene->getMesh(idx));
	}
}

void Node::setupNodeAnimation(aiNodeAnim* anim, double tick_par_second) {
	this->tick_par_second = tick_par_second;

	map<double, aiVector3D> translates;
	map<double, aiQuaternion> rotations;
	map<double, aiVector3D> scales;
	set<double> time_keys;

	for (int i = 0; i < anim->mNumPositionKeys; i++) {
		const aiVectorKey& k = anim->mPositionKeys[i];
		time_keys.insert(k.mTime);
		translates[k.mTime] = k.mValue;
	}

	for (int i = 0; i < anim->mNumRotationKeys; i++) {
		const aiQuatKey& k = anim->mRotationKeys[i];
		time_keys.insert(k.mTime);
		rotations[k.mTime] = k.mValue;
	}

	for (int i = 0; i < anim->mNumScalingKeys; i++) {
		const aiVectorKey& k = anim->mScalingKeys[i];
		time_keys.insert(k.mTime);
		scales[k.mTime] = k.mValue;
	}

	aiVector3D translate;
	aiQuaternion rotation;
	aiVector3D scale;

	set<double>::iterator it = time_keys.begin();
	while (it != time_keys.end()) {
		double key = *it;

		translate = translates.lower_bound(key)->second;
		rotation = rotations.lower_bound(key)->second;
		scale = scales.lower_bound(key)->second;

		ofVec3f s(scale.x, scale.y, scale.z);
		if (s.lengthSquared() < 0.0001) {
			s.set(1);
		}
		
		ofMatrix4x4 m;
		m.glTranslate(translate.x, translate.y, translate.z);
		m.glRotate(
			ofQuaternion(rotation.x, rotation.y, rotation.z, rotation.w));
		m.glScale(s);
		
		animation[key] = m;

		it++;
	}
}

void Node::updateGlobalMatrixCache() {
	if (parent == NULL) {
		global_matrix_cache = matrix;
	} else {
		global_matrix_cache = matrix * parent->global_matrix_cache;
	}
}

OFX_ASSIMP_END_NAMESPACE