#include "Mesh.h"

#include "Scene.h"
#include "Skin.h"

OFX_ASSIMP_BEGIN_NAMESPACE

Mesh::Mesh(Scene* scene, aiMesh* assimp_mesh) {
	material = &scene->resource->materials[assimp_mesh->mMaterialIndex];
	setupMesh(scene, assimp_mesh);
	setupSkin(scene, assimp_mesh);
}

Mesh::~Mesh() {
}

void Mesh::update() {
	if (skin) skin->update();
}

void Mesh::draw(ofPolyRenderMode renderType) {
	material->begin(renderType);

	ofMesh *m;
	if (skin) {
		m = &skin->getAnimatedMesh();
	} else {
		m = &mesh;
	}
	
	switch (renderType) {
		case OF_MESH_FILL:
			m->drawFaces();
			break;
		case OF_MESH_POINTS:
			m->drawVertices();
			break;
		case OF_MESH_WIREFRAME:
			m->drawWireframe();
			break;
	}

	material->end();
}

void Mesh::debugDraw() {
	ofMesh *m;
	if (skin) {
		m = &skin->getAnimatedMesh();
	} else {
		m = &mesh;
	}

	m->drawWireframe();
}

void Mesh::setupMesh(Scene* scene, aiMesh* assimp_mesh) {
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

void Mesh::setupSkin(Scene* scene, aiMesh* assimp_mesh) {
	if (assimp_mesh->mNumBones == 0) return;
	
	Skin *o = new Skin(scene, assimp_mesh, mesh);
	skin = Skin::Ref(o);
}

OFX_ASSIMP_END_NAMESPACE