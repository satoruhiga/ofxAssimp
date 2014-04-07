#include "Node.h"

#include "Scene.h"
#include "Mesh.h"

OFX_ASSIMP_BEGIN_NAMESPACE

Node::Node(Scene* scene, aiNode* node, Node* parent)
	: node(node)
	, parent(parent) {
	name = node->mName.data;

	setupMesh(scene, node);

	if (parent) {
		parent->children.push_back(this);
	}

	matrix = toOF(node->mTransformation);
	updateGlobalMatrixCache();
}

void Node::update() {
	for (int i = 0; i < meshes.size(); i++) {
		meshes[i]->update();
	}
}

void Node::draw() {
	if (meshes.empty()) return;

	ofPushMatrix();
	{
		ofMultMatrix(global_rigid_transform);
		for (int i = 0; i < meshes.size(); i++) {
			meshes[i]->draw();
		}
	}
	ofPopMatrix();
}

void Node::debugDraw() {
	if (parent) {
		ofPushMatrix();
		{
			ofMultMatrix(parent->global_matrix_cache);
			ofLine(ofVec3f(0, 0, 0), matrix.getTranslation());
		}
		ofPopMatrix();
	}

	ofPushMatrix();
	{
		ofMultMatrix(global_rigid_transform);

		for (int i = 0; i < meshes.size(); i++) {
			meshes[i]->debugDraw();
		}
	}
	ofPopMatrix();

	ofPushMatrix();
	{
		ofMultMatrix(global_matrix_cache);

		ofDrawAxis(0.2);
		ofPushStyle();
		{
			ofSetColor(255);

			ofNoFill();
			ofDrawBox(0.2);
			ofDrawBitmapString(name, 0, 0);
		}
		ofPopStyle();
	}
	ofPopMatrix();
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

	aiVector3D position;
	aiQuaternion rotation;
	aiVector3D scaling;

	node->mTransformation.Decompose(scaling, rotation, position);

	set<double>::iterator it = time_keys.begin();
	while (it != time_keys.end()) {
		double key = *it;

		if (translates.find(key) != translates.end()) {
			position = translates[key];
		}

		if (rotations.find(key) != rotations.end()) {
			rotation = rotations[key];
		}

		if (scales.find(key) != scales.end()) {
			scaling = scales[key];
		}

		ofMatrix4x4 m;
		m.glTranslate(position.x, position.y, position.z);
		m.glRotate(
			ofQuaternion(rotation.x, rotation.y, rotation.z, rotation.w));
		m.glScale(scaling.x, scaling.y, scaling.z);

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

	global_rigid_transform.makeIdentityMatrix();
	global_rigid_transform.setTranslation(global_matrix_cache.getTranslation());
	global_rigid_transform.setRotate(global_matrix_cache.getRotate());
}

void Node::updateNodeAnimation(float sec) {
	for (int i = 0; i < meshes.size(); i++) {
		meshes[i]->update();
	}

	if (animation.empty()) return;

	map<double, ofMatrix4x4>::iterator it =
		animation.lower_bound(sec * tick_par_second);
	if (it == animation.end()) return;

	matrix = it->second;
}

void Node::updateNodeAnimationRecursive(Node* node, float sec) {
	node->updateNodeAnimation(sec);
	node->updateGlobalMatrixCache();

	for (int i = 0; i < node->children.size(); i++) {
		Node* child = node->children[i];
		updateNodeAnimationRecursive(child, sec);
	}
}

void Node::setupMesh(Scene* scene, aiNode* node) {
	for (int i = 0; i < node->mNumMeshes; i++) {
		unsigned int mesh_id = node->mMeshes[i];
		const aiScene* s = scene->get();
		aiMesh* m = s->mMeshes[mesh_id];

		Mesh::Ref o = Mesh::Ref(new Mesh(scene, m));
		meshes.push_back(o);
	}
}

OFX_ASSIMP_END_NAMESPACE