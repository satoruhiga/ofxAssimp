#include "ofxAssimp.h"

using namespace ofxAssimp;

static ofMatrix4x4 toOF(const aiMatrix4x4 &m)
{
	return ofMatrix4x4(m.a1, m.b1, m.c1, m.d1,
					   m.a2, m.b2, m.c2, m.d2,
					   m.a3, m.b3, m.c3, m.d3,
					   m.a4, m.b4, m.c4, m.d4);
}

static ofFloatColor toOF(const aiColor4D &c)
{
	return ofFloatColor(c.r, c.g, c.b, c.a);
}

#pragma mark - Material

void Material::begin(ofPolyRenderMode renderType)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
	glEnable(GL_NORMALIZE);
	
	ofPushStyle();
	
	if (hasGlMaterial)
		material.begin();
	
	if (diffuseTex.isAllocated())
		diffuseTex.bind();
	
	ofEnableBlendMode(blendMode);
	
	if (twoSided)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
	
	glPolygonMode(GL_FRONT_AND_BACK, ofGetGLPolyMode(renderType));
}

void Material::end()
{
	if (diffuseTex.isAllocated())
		diffuseTex.unbind();

	if (hasGlMaterial)
		material.end();
	
	ofPopStyle();
	
	glPopClientAttrib();
	glPopAttrib();
}

#pragma mark - Joint

Joint::Joint(Scene* scene, Mesh *mesh, aiBone *bone) : scene(scene), mesh(mesh), bone(bone)
{
	name = bone->mName.data;
	
	static float skin_color_hue = 0;
	skinColor = ofFloatColor::fromHsb(skin_color_hue, 1, 1);
	skin_color_hue += 0.3;

	while (skin_color_hue > 1) skin_color_hue -= 1;
	
	node = scene->getNodeByName(name);
	assert(node);
}

void Joint::updateJointColor(vector<ofFloatColor> &colors)
{
	for (int i = 0; i < bone->mNumWeights; i++)
	{
		aiVertexWeight &w = bone->mWeights[i];
		if (w.mWeight == 0) continue;
		
		colors[w.mVertexId] += skinColor * w.mWeight;
	}
}

void Joint::updateJointTransform(const vector<ofVec3f> &verts_in,
								 vector<ofVec3f> &verts_out,
								 const vector<ofVec3f> &norms_in,
vector<ofVec3f> &norms_out)
{
	ofMatrix4x4 m = node->getBoneMatrix();
	ofMatrix4x4 r = m.getRotate();
	
	if (norms_in.size())
	{
		for (int i = 0; i < bone->mNumWeights; i++)
		{
			aiVertexWeight &w = bone->mWeights[i];
			if (w.mWeight == 0) continue;
			
			verts_out[w.mVertexId] += m.preMult(verts_in[w.mVertexId]) * w.mWeight;
			norms_out[w.mVertexId] += m.preMult(norms_in[w.mVertexId]) * w.mWeight;
		}
	}
	else
	{
		for (int i = 0; i < bone->mNumWeights; i++)
		{
			aiVertexWeight &w = bone->mWeights[i];
			if (w.mWeight == 0) continue;
			
			verts_out[w.mVertexId] += m.preMult(verts_in[w.mVertexId]) * w.mWeight;
		}
	}
}

void Joint::setupGlobalJointPosition()
{
	Node *node = scene->getNodeByName(name);
	if (node->getParent())
	{
		ofMatrix4x4 m = node->getParent()->getGlobalTransformMatrix().getInverse();
		m = toOF(bone->mOffsetMatrix.Inverse()) * m;
		node->setTransformMatrix(m);
	}
}

#pragma mark - Node

Node::Node(Scene *scene, aiNode *node) : scene(scene), node(node), ofNode()
{
	name = node->mName.data;
	setTransformMatrix(toOF(node->mTransformation));
}

void Node::setupInitialTransform()
{
	initialRotationInv = getGlobalOrientation().inverse();
	initialTransformInv = getGlobalTransformMatrix().getInverse();
}

ofMatrix4x4 Node::getBoneMatrix() const
{
	return initialTransformInv * getGlobalTransformMatrix();
}

void Node::draw(bool draw_global)
{
	Node *p = (Node*)getParent();
	if (p)
	{
		p->transformGL();
		ofLine(ofVec3f(0, 0, 0), getPosition());
		p->restoreTransformGL();
	}
	
	transformGL();
	
	if (draw_global)
		ofMultMatrix(initialRotationInv);
	
	ofDrawAxis(0.5);
	restoreTransformGL();
}

#pragma mark - Mesh

Mesh::Mesh(Scene* scene, aiMesh *assimp_mesh) : scene(scene), assimp_mesh(assimp_mesh), visible(true)
{
	setupMesh();
	setupSkeleton();
}

Mesh::~Mesh()
{
	map<string, Joint*>::iterator it = joints.begin();
	while (it != joints.end())
	{
		delete it->second;
		it++;
	}
	joints.clear();
}

void Mesh::update()
{
	if (!visible) return;
	
	mesh = originalMesh;
	
	{
		vector<ofVec3f> vert = originalMesh.getVertices();
		vector<ofVec3f> norm = originalMesh.getNormals();
		
		mesh.getVertices().assign(vert.size(), ofVec3f(0));
		mesh.getNormals().assign(norm.size(), ofVec3f(0));
		
		map<string, Joint*>::iterator it = joints.begin();
		while (it != joints.end())
		{
			Joint *o = it->second;
			o->updateJointTransform(vert, mesh.getVertices(),
									norm, mesh.getNormals());
			it++;
		}
		
		for (int i = 0; i < mesh.getNormals().size(); i++)
		{
			mesh.getNormals()[i].normalize();
		}
	}
}

void Mesh::draw(ofPolyRenderMode renderType)
{
	if (!visible) return;
	
	Material *m = scene->resource.materials[assimp_mesh->mMaterialIndex];
	
	m->begin(renderType);
	
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
	
	m->end();
}

void Mesh::debugDraw()
{
	if (!visible) return;
	
	{
		vector<ofFloatColor>& colors = mesh.getColors();
		
		if (colors.size() == 0)
			colors.resize(mesh.getNumVertices(), ofFloatColor(0, 0, 0, 1));
		
		map<string, Joint*>::iterator it = joints.begin();
		while (it != joints.end())
		{
			Joint *o = it->second;
			o->updateJointColor(colors);
			it++;
		}
	}

	mesh.draw();
}

void Mesh::setupMesh()
{
	name = assimp_mesh->mName.data;
	
	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_TRIANGLES);
	
	assert(assimp_mesh->mNumVertices > 0);
	mesh.addVertices((const ofVec3f*)assimp_mesh->mVertices, assimp_mesh->mNumVertices);
	
	for (int i = 0; i < assimp_mesh->mNumFaces; i++)
	{
		aiFace &face = assimp_mesh->mFaces[i];
		assert(face.mNumIndices == 3);
		
		for (int n = 0; n < face.mNumIndices; n++)
			mesh.addIndex(face.mIndices[n]);
	}
	
	if (assimp_mesh->HasNormals())
		mesh.addNormals((const ofVec3f*)assimp_mesh->mNormals, assimp_mesh->mNumVertices);
	
	if (assimp_mesh->GetNumUVChannels() > 0)
	{
		Material *mtl = scene->resource.materials.at(assimp_mesh->mMaterialIndex);
		
		if (mtl->diffuseTex.isAllocated())
		{
			ofTexture &tex = mtl->diffuseTex;
			for (int i = 0; i  < assimp_mesh->mNumVertices; i++)
			{
				aiVector3D tc = assimp_mesh->mTextureCoords[0][i];
				ofVec2f tcp = tex.getCoordFromPercent(tc.x, tc.y);
				mesh.addTexCoord(tcp);
			}
		}
		else
		{
			for (int i = 0; i  < assimp_mesh->mNumVertices; i++)
			{
				aiVector3D tc = assimp_mesh->mTextureCoords[0][i];
				mesh.addTexCoord(ofVec2f(tc.x, tc.y));
			}
		}
	}
	
	if (assimp_mesh->GetNumColorChannels() > 0)
	{
		mesh.addColors((const ofFloatColor*)assimp_mesh->mColors, assimp_mesh->mNumVertices);
	}
	
	originalMesh = mesh;
}

void Mesh::setupSkeleton()
{
	// create joints
	for (int i = 0; i < assimp_mesh->mNumBones; i++)
	{
		aiBone *bone = assimp_mesh->mBones[i];
		joints[bone->mName.data] = new Joint(scene, this, bone);
	}
	
	{
		map<string, Joint*>::iterator it = joints.begin();
		while (it != joints.end())
		{
			Joint *o = it->second;
			o->setupGlobalJointPosition();
			it++;
		}
	}
}


#pragma mark - Scene

Node* Scene::nodeSetupVisiter(Scene *s, aiNode *node)
{
	Node *n = new Node(s, node);
	s->nodes[n->getName()] = n;
	s->nodeNames.push_back(n->getName());
	
	for (int i = 0; i < node->mNumChildren; i++)
	{
		aiNode *o = node->mChildren[i];
		Node *child = nodeSetupVisiter(s, o);
		child->setParent(*n);
	}
	
	return n;
}

bool Scene::load(string path, bool optimize)
{
	file.open(path);
	return load(file.readToBuffer(), optimize, file.getExtension().c_str());
}

bool Scene::load(const ofBuffer &buffer, bool optimize, const char* extension)
{
	unload();
	
	unsigned int flags = aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_Triangulate | aiProcess_FlipUVs;
	
	if (optimize)
		flags |=  aiProcess_ImproveCacheLocality | aiProcess_OptimizeGraph |
		aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices |
		aiProcess_RemoveRedundantMaterials;
	
	scene = aiImportFileFromMemory(buffer.getBinaryBuffer(), buffer.size(), flags, extension);
	assert(scene);
	
	setupResources();
	setupMeshs();
}

void Scene::unload()
{
	if (scene)
	{
		aiReleaseImport(scene);
		scene = NULL;
	}
	
	{
		map<string, Node*>::iterator it = nodes.begin();
		while (it != nodes.end())
		{
			delete it->second;
			it++;
		}
		nodes.clear();
	}

	{
		for (int i = 0; i < meshs.size(); i++)
		{
			delete meshs[i];
		}
		meshs.clear();
	}
}

void Scene::dumpScene()
{
	cout << "==== Mesh ====" << endl;
	for (int i = 0; i < meshs.size(); i++)
	{
		printf("%03i: %s\n", i, meshs[i]->getName().c_str());
	}

	cout << endl;
	
	cout << "==== Node ====" << endl;
	for (int i = 0; i < nodeNames.size(); i++)
	{
		printf("%03i: %s\n", i, nodeNames[i].c_str());
	}
}

void Scene::update()
{
	for (int i = 0; i < meshs.size(); i++)
	{
		meshs[i]->update();
	}
}

void Scene::draw()
{
	for (int i = 0; i < meshs.size(); i++)
	{
		meshs[i]->draw();
	}
}

void Scene::debugDraw(bool draw_global)
{
	{
		for (int i = 0; i < meshs.size(); i++)
		{
			meshs[i]->debugDraw();
		}
	}

	{
		map<string, Node*>::iterator it = nodes.begin();
		while (it != nodes.end())
		{
			it->second->draw(draw_global);
			it++;
		}
	}
}

Mesh* Scene::getMesh(size_t index)
{
	return meshs.at(index);
}

Node* Scene::getNodeByName(const string& name)
{
	if (nodes.find(name) == nodes.end()) return NULL;
	return nodes[name];
}

void Scene::setupResources()
{
	for (int i = 0; i < scene->mNumMaterials; i++)
	{
		aiMaterial *mtl = scene->mMaterials[i];
		resource.materials.push_back(new Material);
		
		Material& m = *resource.materials.back();
		
		aiColor4D dcolor, scolor, acolor, ecolor;
		
		if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &dcolor))
		{
			m.material.setDiffuseColor(toOF(dcolor));
			m.hasGlMaterial = true;
		}
		
		if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &scolor))
		{
			m.material.setSpecularColor(toOF(scolor));
			m.hasGlMaterial = true;
		}
		
		if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &acolor))
		{
			m.material.setAmbientColor(toOF(acolor));
			m.hasGlMaterial = true;
		}
		
		if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &ecolor))
		{
			m.material.setEmissiveColor(toOF(ecolor));
			m.hasGlMaterial = true;
		}
		
		float shininess;
		if (AI_SUCCESS == aiGetMaterialFloat(mtl, AI_MATKEY_SHININESS, &shininess))
		{
			m.material.setShininess(shininess);
			m.hasGlMaterial = true;
		}
		
		int blendMode;
		if (AI_SUCCESS == aiGetMaterialInteger(mtl, AI_MATKEY_BLEND_FUNC, &blendMode))
		{
			if (blendMode == aiBlendMode_Default)
			{
				m.blendMode = OF_BLENDMODE_ALPHA;
			}
			else
			{
				m.blendMode = OF_BLENDMODE_ADD;
			}
		}
		
		unsigned int max = 1;
		int two_sided;
		if ((AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_TWOSIDED, &two_sided, &max)) && two_sided)
			m.twoSided = true;
		else
			m.twoSided = false;
		

		int texIndex = 0;
		aiString texPath;
		
		if (AI_SUCCESS == mtl->GetTexture(aiTextureType_DIFFUSE, texIndex, &texPath))
		{
			string modelFolder = file.getEnclosingDirectory();
			string relTexPath = ofFilePath::getEnclosingDirectory(texPath.data, false);
			string texFile = ofFilePath::getFileName(texPath.data);
			string realPath = modelFolder + relTexPath  + texFile;
			
			if (!ofFile::doesFileExist(realPath) || !ofLoadImage(m.diffuseTex, realPath))
			{
				ofLog(OF_LOG_ERROR, string("error loading image ") + file.getFileName() + " " + realPath);
			}
		}
	}
}

void Scene::setupMeshs()
{
	nodeSetupVisiter(this, scene->mRootNode);
	
	for (int i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh *mesh = scene->mMeshes[i];
		meshs.push_back(new Mesh(this, mesh));
	}

	map<string, Node*>::iterator it = nodes.begin();
	while (it != nodes.end())
	{
		it->second->setupInitialTransform();
		it++;
	}
}

