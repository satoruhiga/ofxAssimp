#include "Skin.h"

#include "Scene.h"

OFX_ASSIMP_BEGIN_NAMESPACE

Skin::Skin(Scene* scene, aiMesh* assimp_mesh, ofMesh& mesh) {
	this->mesh = mesh;
	this->animated_mesh = mesh;

	setupVertexWeights(scene, assimp_mesh);
	setupJoints(scene, assimp_mesh);
//	setupShader();
}

void Skin::setupVertexWeights(Scene* scene, aiMesh* mesh) {
	int num_vertexes = mesh->mNumVertices;

	vertex_weights.resize(num_vertexes);

	for (int i = 0; i < mesh->mNumBones; i++) {
		aiBone* bone = mesh->mBones[i];

		for (int n = 0; n < bone->mNumWeights; n++) {
			aiVertexWeight& o = bone->mWeights[n];
			vertex_weights[o.mVertexId].setWeight(i, o.mWeight);
		}
	}
}

//void Skin::setupShader() {
//	skin_shader.unload();
//	skin_shader.load("skin.vs", "skin.fs");
//
//	skin_shader.begin();
//	skin_shader.setAttribute4fv("bone_ids", (float*)vertex_weights[0].bone_ids, sizeof(float) * 8);
//	skin_shader.setAttribute4fv("weights", (float*)vertex_weights[0].weights,
//								sizeof(float) * 8);
//	skin_shader.end();
//}

void Skin::setupJoints(Scene* scene, aiMesh* mesh) {
	bone_matrixes.resize(mesh->mNumBones);
	offset_matrixes.resize(mesh->mNumBones);

	for (int i = 0; i < mesh->mNumBones; i++) {
		aiBone* bone = mesh->mBones[i];
		Node::Ref o = scene->getNodeByName(bone->mName.data);
		nodes.push_back(o.get());

		offset_matrixes[i] = toOF(bone->mOffsetMatrix);
	}
}

void Skin::update() {
	for (int i = 0; i < nodes.size(); i++) {
		bone_matrixes[i] = offset_matrixes[i] * nodes[i]->getGlobalMatrix();
	}

	const vector<ofVec3f>& src_vert = mesh.getVertices();
	vector<ofVec3f>& dst_vert = animated_mesh.getVertices();
	for (int i = 0; i < src_vert.size(); i++) {
		VertexWeight& w = vertex_weights[i];
		const ofVec3f& v = src_vert[i];
		ofVec3f o;

		for (int n = 0; n < 4; n++) {
			float weight = w.weights[n];
			if (weight == 0) break;

			int N = w.bone_ids[n];
			o += bone_matrixes[N].preMult(v) * weight;
		}

		dst_vert[i] = o;
	}
}

//void Skin::begin() {
//	skin_shader.begin();
//
//	GLint loc =
//		glGetUniformLocation(skin_shader.getProgram(), "bone_transforms");
//	if (loc >= 0) {
//		glUniformMatrix4fv(loc, bone_matrixes.size(), GL_FALSE,
//						   bone_matrixes[0].getPtr());
//	}
//
//	int err = glGetError();
//	if (err != GL_NO_ERROR) {
//		cout << gluErrorString(err) << endl;
//	}
//}
//
//void Skin::end() { skin_shader.end(); }

OFX_ASSIMP_END_NAMESPACE