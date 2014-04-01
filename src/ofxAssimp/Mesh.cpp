#include "Mesh.h"

#include "Joint.h"
#include "Scene.h"

OFX_ASSIMP_BEGIN_NAMESPACE

Mesh::Mesh(Scene* scene, aiMesh* assimp_mesh)
	: scene(scene)
	, assimp_mesh(assimp_mesh) {
	setupMesh();
	setupSkeleton();
}

Mesh::~Mesh() {
	joints.clear();
}


void Mesh::update() {
//	mesh = originalMesh;
//
//	{
//		vector<ofVec3f> vert = originalMesh.getVertices();
//		vector<ofVec3f> norm = originalMesh.getNormals();
//
//		mesh.getVertices().assign(vert.size(), ofVec3f(0));
//		mesh.getNormals().assign(norm.size(), ofVec3f(0));
//
//		map<string, Joint::Ref>::iterator it = joints.begin();
//		while (it != joints.end()) {
//			Joint::Ref o = it->second;
//			o->updateJointTransform(vert, mesh.getVertices(), norm,
//									mesh.getNormals());
//			it++;
//		}
//
//		for (int i = 0; i < mesh.getNormals().size(); i++) {
//			mesh.getNormals()[i].normalize();
//		}
//	}
}

void Mesh::draw(ofPolyRenderMode renderType) {
	Material& m = scene->resource->materials[assimp_mesh->mMaterialIndex];

	m.begin(renderType);

	switch (renderType) {
		case OF_MESH_FILL:
			mesh.drawFaces();
			break;
		case OF_MESH_POINTS:
			mesh.drawVertices();
			break;
		case OF_MESH_WIREFRAME:
			mesh.drawWireframe();
			break;
	}

	m.end();
}

void Mesh::debugDraw() {
//	{
//		vector<ofFloatColor>& colors = mesh.getColors();
//
//		if (colors.size() == 0)
//			colors.resize(mesh.getNumVertices(), ofFloatColor(0, 0, 0, 1));
//
//		map<string, Joint::Ref>::iterator it = joints.begin();
//		while (it != joints.end()) {
//			Joint::Ref o = it->second;
//			o->updateJointColor(colors);
//			it++;
//		}
//	}

	mesh.drawWireframe();
}

void Mesh::setupMesh() {
	name = assimp_mesh->mName.data;

	mesh.clear();
	mesh.setMode(OF_PRIMITIVE_TRIANGLES);

	assert(assimp_mesh->mNumVertices > 0);
	mesh.addVertices((const ofVec3f*)assimp_mesh->mVertices,
					 assimp_mesh->mNumVertices);

	for (int i = 0; i < assimp_mesh->mNumFaces; i++) {
		aiFace& face = assimp_mesh->mFaces[i];
		assert(face.mNumIndices == 3);

		for (int n = 0; n < face.mNumIndices; n++)
			mesh.addIndex(face.mIndices[n]);
	}

	if (assimp_mesh->HasNormals())
		mesh.addNormals((const ofVec3f*)assimp_mesh->mNormals,
						assimp_mesh->mNumVertices);

	if (assimp_mesh->GetNumUVChannels() > 0) {
		Material& mtl =
			scene->resource->materials.at(assimp_mesh->mMaterialIndex);

		if (mtl.diffuseTex.isAllocated()) {
			ofTexture& tex = mtl.diffuseTex;
			for (int i = 0; i < assimp_mesh->mNumVertices; i++) {
				aiVector3D tc = assimp_mesh->mTextureCoords[0][i];
				ofVec2f tcp = tex.getCoordFromPercent(tc.x, tc.y);
				mesh.addTexCoord(tcp);
			}
		} else {
			for (int i = 0; i < assimp_mesh->mNumVertices; i++) {
				aiVector3D tc = assimp_mesh->mTextureCoords[0][i];
				mesh.addTexCoord(ofVec2f(tc.x, tc.y));
			}
		}
	}

	if (assimp_mesh->GetNumColorChannels() > 0) {
		mesh.addColors((const ofFloatColor*)assimp_mesh->mColors,
					   assimp_mesh->mNumVertices);
	}
}

void Mesh::setupSkeleton() {
	// create joints
	for (int i = 0; i < assimp_mesh->mNumBones; i++) {
		aiBone* bone = assimp_mesh->mBones[i];
		Joint::Ref o = Joint::Ref(new Joint(scene, this, bone));
		joints[bone->mName.data] = o;
	}

	{
		map<string, Joint::Ref>::iterator it = joints.begin();
		while (it != joints.end()) {
			Joint::Ref o = it->second;
			o->setupGlobalJointPosition();
			it++;
		}
	}
}

OFX_ASSIMP_END_NAMESPACE