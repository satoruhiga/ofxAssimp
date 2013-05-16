#pragma once

#include "ofMain.h"

#include "assimp/scene.h"
#include "assimp/config.h"
#include "assimp/postprocess.h"
#include "assimp/cimport.h"

namespace ofxAssimp
{
	class Scene;
	class Mesh;
	class Node;
	class Joint;
	class Resource;
	class Material;
}

class ofxAssimp::Material
{
public:
	
	Material() : hasGlMaterial(false), twoSided(true), blendMode(OF_BLENDMODE_ALPHA) {}
	
	bool hasGlMaterial;
	ofMaterial material;
	ofBlendMode blendMode;
	
	ofTexture diffuseTex;
	
	bool twoSided;
	
	void begin(ofPolyRenderMode renderType);
	void end();
};

class ofxAssimp::Resource
{
public:
	
	vector<Material*> materials;
};

class ofxAssimp::Joint
{
	friend class Mesh;
	
public:
	
	Joint(Scene* scene, Mesh *mesh, aiBone *bone);
	
	void updateJointColor(vector<ofFloatColor> &colors);
	void updateJointTransform(const vector<ofVec3f> &verts_in,
							  vector<ofVec3f> &verts_out,
							  const vector<ofVec3f> &norms_in,
							  vector<ofVec3f> &norms_out);
	
	aiBone* get() { return bone; }
	const aiBone* get() const { return bone; }
	
protected:
	
	aiBone *bone;

	Scene *scene;
	Mesh *mesh;
	Node *node;
	
	string name;
	
	ofFloatColor skinColor;
	
	void setupGlobalJointPosition();
};

class ofxAssimp::Node : public ofNode
{
	friend class Scene;
	
public:
	
	Node(Scene *scene, aiNode *node);
	
	void draw(bool draw_global);
	
	const string& getName() const { return name; }
	
	inline const ofMatrix4x4& getInitialTransformInv() const { return initialTransformInv; }
	inline const ofMatrix4x4& getInitialRotationInv() const { return initialRotationInv; }
	
	ofMatrix4x4 getBoneMatrix() const;
	
	aiNode* get() { return node; }
	const aiNode* get() const { return node; }
	
private:
	
	string name;
	
	Scene *scene;
	aiNode *node;
	
	ofMatrix4x4 initialRotationInv;
	ofMatrix4x4 initialTransformInv;
	
	void setupInitialTransform();
};

class ofxAssimp::Mesh
{
	friend class Joint;
	
public:
	
	Mesh(Scene* scene, aiMesh *assimp_mesh);
	~Mesh();
	
	void update();
	
	void draw(ofPolyRenderMode renderType = OF_MESH_FILL);
	void debugDraw();
	
	void setVisible(bool yn = true) { visible = yn; }
	bool getVisible() const { return visible; }
	
	const string& getName() { return name; }
	
	ofMesh getMesh() { return mesh; }
	const ofMesh& getMesh() const { return mesh; }
	
protected:
	
	Scene *scene;
	aiMesh *assimp_mesh;
	
	string name;
	
	ofMesh originalMesh;
	ofMesh mesh;
	
	map<string, Joint*> joints;
	
	bool visible;
	
	void setupMesh();
	void setupSkeleton();
};

class ofxAssimp::Scene
{
	friend class Node;
	friend class Joint;
	friend class Mesh;
	
public:
	
	Scene() : scene(NULL) {}
	
	bool load(string path, bool optimize = false);
	bool load(const ofBuffer &buffer, bool optimize = false, const char* extension = "");
	void unload();
	
	void dumpScene();
	
	void update();
	
	void draw();
	void debugDraw(bool draw_global = true);
	
	inline const aiScene* get() const { return scene; }
	
	size_t getNumMesh() const { return meshs.size(); }
	Mesh* getMesh(size_t index);
	
	inline const vector<string>& getNodeNames() const { return nodeNames; }
	Node* getNodeByName(const string& name);
	
protected:
	
	const aiScene *scene;
	
	ofFile file;
	
	vector<Mesh*> meshs;
	
	vector<string> nodeNames;
	map<string, Node*> nodes;
	
	Resource resource;
	
	static Node* nodeSetupVisiter(Scene *s, aiNode *node);
	
	void setupResources();
	void setupMeshs();
};
