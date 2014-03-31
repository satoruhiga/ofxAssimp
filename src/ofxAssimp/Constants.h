#pragma once

#include "ofMain.h"

#include <assimp/scene.h>
#include <assimp/config.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

#define OFX_ASSIMP_BEGIN_NAMESPACE namespace ofx { namespace Assimp {
#define OFX_ASSIMP_END_NAMESPACE } }

OFX_ASSIMP_BEGIN_NAMESPACE

static ofMatrix4x4 toOF(const aiMatrix4x4& m) {
	return ofMatrix4x4(m.a1, m.b1, m.c1, m.d1, m.a2, m.b2, m.c2, m.d2, m.a3,
					   m.b3, m.c3, m.d3, m.a4, m.b4, m.c4, m.d4);
}

static ofFloatColor toOF(const aiColor4D& c) {
	return ofFloatColor(c.r, c.g, c.b, c.a);
}

class Node;
class Joint;
class Mesh;
class Resource;

OFX_ASSIMP_END_NAMESPACE

namespace ofxAssimp = ofx::Assimp;