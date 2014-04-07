#include "Resource.h"

#include "Scene.h"

OFX_ASSIMP_BEGIN_NAMESPACE

Resource::Resource(Scene* s, ofFile& file) {
	const aiScene *scene = s->get();
	
	for (int i = 0; i < scene->mNumMaterials; i++) {
		aiMaterial* mtl = scene->mMaterials[i];
		materials.push_back(Material());
		
		Material& m = materials.back();
		
		aiColor4D dcolor, scolor, acolor, ecolor;
		
		if (AI_SUCCESS ==
			aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &dcolor)) {
			m.material.setDiffuseColor(toOF(dcolor));
			m.hasGlMaterial = true;
		}
		
		if (AI_SUCCESS ==
			aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &scolor)) {
			m.material.setSpecularColor(toOF(scolor));
			m.hasGlMaterial = true;
		}
		
		if (AI_SUCCESS ==
			aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &acolor)) {
			m.material.setAmbientColor(toOF(acolor));
			m.hasGlMaterial = true;
		}
		
		if (AI_SUCCESS ==
			aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &ecolor)) {
			m.material.setEmissiveColor(toOF(ecolor));
			m.hasGlMaterial = true;
		}
		
		float shininess;
		if (AI_SUCCESS ==
			aiGetMaterialFloat(mtl, AI_MATKEY_SHININESS, &shininess)) {
			m.material.setShininess(shininess);
			m.hasGlMaterial = true;
		}
		
		int blendMode;
		if (AI_SUCCESS ==
			aiGetMaterialInteger(mtl, AI_MATKEY_BLEND_FUNC, &blendMode)) {
			if (blendMode == aiBlendMode_Default) {
				m.blendMode = OF_BLENDMODE_ALPHA;
			} else {
				m.blendMode = OF_BLENDMODE_ADD;
			}
		}
		
		unsigned int max = 1;
		int two_sided;
		if ((AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_TWOSIDED,
													 &two_sided, &max)) &&
			two_sided)
			m.twoSided = true;
		else
			m.twoSided = false;
		
		int texIndex = 0;
		aiString texPath;
		
		if (AI_SUCCESS ==
			mtl->GetTexture(aiTextureType_DIFFUSE, texIndex, &texPath)) {
			string modelFolder = file.getEnclosingDirectory();
			string relTexPath =
			ofFilePath::getEnclosingDirectory(texPath.data, false);
			string texFile = ofFilePath::getFileName(texPath.data);
			string realPath = modelFolder + relTexPath + texFile;
			
			if (!ofFile::doesFileExist(realPath) ||
				!ofLoadImage(m.diffuseTex, realPath)) {
				ofLog(OF_LOG_ERROR, string("error loading image ") +
					  file.getFileName() + " " + realPath);
			}
		}
	}

}

OFX_ASSIMP_END_NAMESPACE