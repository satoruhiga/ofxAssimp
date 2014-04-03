#pragma once

#include "Constants.h"

OFX_ASSIMP_BEGIN_NAMESPACE

class Node;
class Scene;
class Mesh;

#define OFX_ASSIMP_NUM_BONE_PER_VERTEX 4

class Skin {
public:
	
	typedef ofPtr<Skin> Ref;
	
	Skin(Scene* scene, aiMesh* assimp_mesh, ofMesh& mesh);

	void update();
	
	void begin();
	void end();
	
	ofMesh& getAnimatedMesh() { return animated_mesh; }
	
private:
	
	struct VertexWeight {
		float bone_ids[OFX_ASSIMP_NUM_BONE_PER_VERTEX];
		float weights[OFX_ASSIMP_NUM_BONE_PER_VERTEX];
		
		VertexWeight() {
			std::fill(bone_ids, bone_ids + OFX_ASSIMP_NUM_BONE_PER_VERTEX, 0);
			std::fill(weights, weights + OFX_ASSIMP_NUM_BONE_PER_VERTEX, 0);
		}
		
		void setWeight(int bone_id, float weight) {
			for (int i = 0; i < OFX_ASSIMP_NUM_BONE_PER_VERTEX; i++) {
				if (weights[i] == 0) {
					bone_ids[i] = bone_id;
					weights[i] = weight;
					return;
				}
			}
			assert(false);
		}
	};
	
	ofMesh mesh, animated_mesh;

	vector<VertexWeight> vertex_weights;
	void setupVertexWeights(Scene* scene, aiMesh* mesh);
	
	ofShader skin_shader;
	void setupShader();
	
	vector<Node*> nodes;
	vector<ofMatrix4x4> bone_matrixes;
	vector<ofMatrix4x4> offset_matrixes;
	ofMatrix4x4 root_node_transform_inv;
	void setupJoints(Scene* scene, aiMesh* mesh);
};

OFX_ASSIMP_END_NAMESPACE
