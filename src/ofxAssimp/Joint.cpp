#include "Joint.h"

#include "Node.h"

OFX_ASSIMP_BEGIN_NAMESPACE

Joint::Joint(Scene* scene, Mesh* mesh, aiBone* bone)
: scene(scene)
, mesh(mesh)
, bone(bone) {
	name = bone->mName.data;
	
	static float skin_color_hue = 0;
	skinColor = ofFloatColor::fromHsb(skin_color_hue, 1, 1);
	skin_color_hue += 0.3;
	
	while (skin_color_hue > 1) skin_color_hue -= 1;
	
	node = scene->getNodeByName(name);
	assert(node);
}

void Joint::updateJointColor(vector<ofFloatColor>& colors) {
	for (int i = 0; i < bone->mNumWeights; i++) {
		aiVertexWeight& w = bone->mWeights[i];
		if (w.mWeight == 0) continue;
		
		colors[w.mVertexId] += skinColor * w.mWeight;
	}
}

void Joint::updateJointTransform(const vector<ofVec3f>& verts_in,
						  vector<ofVec3f>& verts_out,
						  const vector<ofVec3f>& norms_in,
						  vector<ofVec3f>& norms_out){
	ofMatrix4x4 m = node->getBoneMatrix();
	ofMatrix4x4 r = m.getRotate();
	
	if (norms_in.size()) {
		for (int i = 0; i < bone->mNumWeights; i++) {
			aiVertexWeight& w = bone->mWeights[i];
			if (w.mWeight == 0) continue;
			
			verts_out[w.mVertexId] +=
			m.preMult(verts_in[w.mVertexId]) * w.mWeight;
			norms_out[w.mVertexId] +=
			m.preMult(norms_in[w.mVertexId]) * w.mWeight;
		}
	} else {
		for (int i = 0; i < bone->mNumWeights; i++) {
			aiVertexWeight& w = bone->mWeights[i];
			if (w.mWeight == 0) continue;
			
			verts_out[w.mVertexId] +=
			m.preMult(verts_in[w.mVertexId]) * w.mWeight;
		}
	}
}

void Joint::setupGlobalJointPosition() {
	Node::Ref node = scene->getNodeByName(name);
	if (node->getParent()) {
		ofMatrix4x4 m =
		node->getParent()->getGlobalTransformMatrix().getInverse();
		m = toOF(bone->mOffsetMatrix.Inverse()) * m;
		node->setTransformMatrix(m);
	}
}

OFX_ASSIMP_END_NAMESPACE