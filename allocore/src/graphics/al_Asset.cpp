#include <algorithm> // min,max
#include "allocore/graphics/al_Asset.hpp"
#include "allocore/graphics/al_Graphics.hpp"

#ifndef USE_ASSIMP3
#define USE_ASSIMP3
#endif

#ifdef USE_ASSIMP3

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/cimport.h"
#include "assimp/types.h"
#include "assimp/matrix4x4.h"

#else

#include "assimp/assimp.h"
#include "assimp/aiTypes.h"
#include "assimp/aiPostProcess.h"
#include "assimp/aiScene.h"
#include "assimp/aiMaterial.h"

#endif

#include <stdio.h>
#include <map>

using namespace al;

Vec4f vec4FromAIColor4D(aiColor4D& v) {
	return Vec4f(v.r, v.g, v.b, v.a);
}

Vec3f vec3FromAIVector3D(aiVector3D& v) {
	return Vec3f(v.x, v.y, v.z);
}

Vec2f vec2FromAIVector3D(aiVector3D& v) {
	return Vec2f(v.x, v.y);
}


void initLogStream() {
	static bool initializedLog = false;
	static struct aiLogStream logStream;
	if (!initializedLog) {
		initializedLog = true;
		// get a handle to the predefined STDOUT log stream and attach
		// it to the logging system. It will be active for all further
		// calls to aiImportFile(Ex) and aiApplyPostProcessing.
		logStream = aiGetPredefinedLogStream(aiDefaultLogStream_STDOUT,NULL);
		aiAttachLogStream(&logStream);
	}
}

int countNodes(const aiNode * n) {
	int count = 1;
	for (unsigned int i=0; i<n->mNumChildren; i++) {
		count += countNodes(n->mChildren[i]);
	}
	return count;
}






class Scene::Node::Impl {
public:
	Impl(const aiNode * node) : node(node) {}
	
	const aiNode * node;
};


class Scene::Impl /*: public SceneNode::Impl*/ {
public:
	Impl(const aiScene * scene) : /* SceneNode::Impl(scene->mRootNode),*/ scene(scene) {
		nodes.resize(countNodes(scene->mRootNode));
		addNode(scene->mRootNode, 0);
	}
	
	~Impl() {
		aiReleaseImport(scene);
	}
	
	int addNode(const aiNode * n, int idx) {
		nodes[idx].mImpl = new Scene::Node::Impl(n);
		nodeMap[n] = idx;
		for (unsigned int i=0; i<n->mNumChildren; i++) {
			idx = addNode(n->mChildren[i], idx+1);
		}
		return idx;
	}
	
	const aiScene * scene;
	
	std::map<const aiNode *, int> nodeMap;
	std::vector<Node> nodes;
};


Scene::Material :: Material() 
:	shading_model(0), 
	two_sided(0), 
	wireframe(0), 
	blend_func(0), 
	shininess(0.25), 
	shininess_strength(1), 
	opacity(1), 
	reflectivity(0), 
	refracti(0), 
	bump_scaling(1),
	diffuse(0.6),
	ambient(0),
	specular(1.),
	emissive(0.),
	transparent(0.),
	reflective(0.)
	{}


Scene::Node :: Node() : mImpl(0) {}
Scene::Node :: ~Node() { if (mImpl) delete mImpl; }

std::string Scene::Node :: name() const {
	return std::string(mImpl->node->mName.data);
}

void Scene::verbose(bool b) {
	aiEnableVerboseLogging(b);
}


Scene * Scene :: import(const std::string& path, ImportPreset preset) {
	//initLogStream();
	int flags=0;
	switch (preset) {
		case FAST:
			flags = aiProcessPreset_TargetRealtime_Fast;
			break;
		case QUALITY:
			flags = aiProcessPreset_TargetRealtime_Quality;
			break;
		case MAX_QUALITY:
			flags = aiProcessPreset_TargetRealtime_MaxQuality;
			break;
		default:
			break;
	}
	const aiScene * scene = aiImportFile(path.c_str(), flags);
	if (scene) {
		Impl * impl = new Impl(scene);
		Scene * s = new Scene(impl);
		return s;
	} else {
		return NULL;
	}

}

Scene :: Scene(Impl * impl) : mImpl(impl) {
	
	mMaterials.resize(materials());
	for (unsigned int i=0; i<materials(); i++) {
		Scene::Material& m = mMaterials[i];
		unsigned int max;
		
		// import materials:
		aiMaterial* mat = mImpl->scene->mMaterials[i];
		aiString szPath;
		
		max = 4;
		aiGetMaterialFloatArray(mat, AI_MATKEY_COLOR_DIFFUSE, m.diffuse.components, &max);
		aiGetMaterialFloatArray(mat, AI_MATKEY_COLOR_AMBIENT, m.ambient.components, &max);
		aiGetMaterialFloatArray(mat, AI_MATKEY_COLOR_SPECULAR, m.specular.components, &max);
		aiGetMaterialFloatArray(mat, AI_MATKEY_COLOR_EMISSIVE, m.emissive.components, &max);
		aiGetMaterialFloatArray(mat, AI_MATKEY_COLOR_TRANSPARENT, m.transparent.components, &max);
		aiGetMaterialFloatArray(mat, AI_MATKEY_COLOR_REFLECTIVE, m.reflective.components, &max);
		aiGetMaterialFloat(mat, AI_MATKEY_OPACITY, &m.opacity);
		aiGetMaterialFloat(mat, AI_MATKEY_SHININESS, &m.shininess);
		aiGetMaterialFloat(mat, AI_MATKEY_SHININESS_STRENGTH, &m.shininess_strength);
		aiGetMaterialFloat(mat, AI_MATKEY_REFLECTIVITY, &m.reflectivity);
		aiGetMaterialFloat(mat, AI_MATKEY_REFRACTI, &m.refracti);
		aiGetMaterialFloat(mat, AI_MATKEY_BUMPSCALING, &m.bump_scaling);
		
		aiGetMaterialInteger(mat, AI_MATKEY_TWOSIDED, &m.two_sided);
		aiGetMaterialInteger(mat, AI_MATKEY_SHADING_MODEL, &m.shading_model);
		aiGetMaterialInteger(mat, AI_MATKEY_ENABLE_WIREFRAME, &m.wireframe);
		aiGetMaterialInteger(mat, AI_MATKEY_BLEND_FUNC, &m.blend_func);
		
		aiGetMaterialString(mat, AI_MATKEY_NAME, &szPath);
		m.name = std::string(szPath.data);
		
		aiGetMaterialString(mat, AI_MATKEY_GLOBAL_BACKGROUND_IMAGE, &szPath);
		m.background = std::string(szPath.data);
		
		if(AI_SUCCESS == aiGetMaterialString(mat, AI_MATKEY_TEXTURE_DIFFUSE(0), &szPath)) {
			m.diffusemap.useTexture = true;
			m.diffusemap.texture = std::string(szPath.data);
		} 
		if(AI_SUCCESS == aiGetMaterialString(mat, AI_MATKEY_TEXTURE_AMBIENT(0), &szPath)) {
			m.ambientmap.useTexture = true;
			m.ambientmap.texture = std::string(szPath.data);
		} 
		if(AI_SUCCESS == aiGetMaterialString(mat, AI_MATKEY_TEXTURE_SPECULAR(0), &szPath)) {
			m.specularmap.useTexture = true;
			m.specularmap.texture = std::string(szPath.data);
		} 
		if(AI_SUCCESS == aiGetMaterialString(mat, AI_MATKEY_TEXTURE_OPACITY(0), &szPath)) {
			m.opacitymap.useTexture = true;
			m.opacitymap.texture = std::string(szPath.data);
		} 
		
		if(AI_SUCCESS == aiGetMaterialString(mat, AI_MATKEY_TEXTURE_EMISSIVE(0), &szPath)) {
			m.emissivemap.useTexture = true;
			m.emissivemap.texture = std::string(szPath.data);
		}
		if(AI_SUCCESS == aiGetMaterialString(mat, AI_MATKEY_TEXTURE_SHININESS(0), &szPath)) {
			m.shininessmap.useTexture = true;
			m.shininessmap.texture = std::string(szPath.data);
		}
		if(AI_SUCCESS == aiGetMaterialString(mat, AI_MATKEY_TEXTURE_LIGHTMAP(0), &szPath)) {
			m.lightmap.useTexture = true;
			m.lightmap.texture = std::string(szPath.data);
		}
		if(AI_SUCCESS == aiGetMaterialString(mat, AI_MATKEY_TEXTURE_NORMALS(0), &szPath)) {
			m.normalmap.useTexture = true;
			m.normalmap.texture = std::string(szPath.data);
		}
		if(AI_SUCCESS == aiGetMaterialString(mat, AI_MATKEY_TEXTURE_HEIGHT(0), &szPath)) {
			m.heightmap.useTexture = true;
			m.heightmap.texture = std::string(szPath.data);
		}
		if(AI_SUCCESS == aiGetMaterialString(mat, AI_MATKEY_TEXTURE_DISPLACEMENT(0), &szPath)) {
			m.displacementmap.useTexture = true;
			m.displacementmap.texture = std::string(szPath.data);
		}
		if(AI_SUCCESS == aiGetMaterialString(mat, AI_MATKEY_TEXTURE_REFLECTION(0), &szPath)) {
			m.reflectionmap.useTexture = true;
			m.reflectionmap.texture = std::string(szPath.data);
		}
	}
}

Scene :: ~Scene() {
	delete mImpl;
}

unsigned int Scene :: meshes() const {
	return mImpl->scene->mNumMeshes;
}

void Scene :: mesh(unsigned int meshIdx, Mesh& mesh) const {
	if(meshIdx < meshes()){
		const aiMesh& amesh = *mImpl->scene->mMeshes[meshIdx];
		
		// Derive primitive type
		auto prim = Graphics::POINTS;
		if(amesh.HasFaces()){
			switch(amesh.mFaces[0].mNumIndices){
				case 2: prim = Graphics::LINES; break;
				case 3: prim = Graphics::TRIANGLES; break;
			}
		}
		mesh.primitive(prim);

		bool hasNormals = amesh.HasNormals();
		bool hasColors = amesh.HasVertexColors(0);
		bool hasTexcoords = amesh.HasTextureCoords(0);

		for(unsigned i=0; i<amesh.mNumVertices; ++i){
			mesh.vertex(vec3FromAIVector3D(amesh.mVertices[i]));
			if(hasColors) mesh.color(vec4FromAIColor4D(amesh.mColors[0][i]));
			if(hasNormals) mesh.normal(vec3FromAIVector3D(amesh.mNormals[i]));
			if(hasTexcoords) mesh.texCoord(vec2FromAIVector3D(amesh.mTextureCoords[0][i]));
		}

		int Nv = mesh.vertices().size(); // since we are appending to mesh

		for(unsigned j=0; j<amesh.mNumFaces; ++j){
			const aiFace& face = amesh.mFaces[j];
			for(unsigned i=0; i<face.mNumIndices; ++i){
				mesh.index(Nv + face.mIndices[i]);
			}
		}
	}
}

const Scene::Material& Scene :: material(unsigned int i) const {
	return mMaterials[i];
}

unsigned int Scene :: meshMaterial(unsigned int i) const {
	if (i < meshes()) {
		aiMesh * amesh = mImpl->scene->mMeshes[i];
		if (amesh) {
			return amesh->mMaterialIndex;
		}
	}
	return 0;
}

std::string Scene :: meshName(unsigned int i) const {
	if (i < meshes()) {
		aiMesh * amesh = mImpl->scene->mMeshes[i];
		if (amesh) {
			return amesh->mName.data;
		}
	}
	return 0;
}

unsigned int Scene :: materials() const {
	return mImpl->scene->mNumMaterials;
}

unsigned int Scene :: textures() const {
	return mImpl->scene->mNumTextures;
}

unsigned int Scene :: nodes() const {
	return mImpl->nodes.size();
}

Scene::Node& Scene :: node(unsigned int i) const {
	return mImpl->nodes[i];
}

#ifdef USE_ASSIMP3
void get_bounding_box_for_node(const aiScene * scene, const struct aiNode* nd, Vec3f& min, Vec3f& max, aiMatrix4x4* trafo) {
    aiMatrix4x4 prev;
#else
void get_bounding_box_for_node(const aiScene * scene, const struct aiNode* nd, Vec3f& min, Vec3f& max, struct aiMatrix4x4* trafo) {
    struct aiMatrix4x4 prev;
#endif
	unsigned int n = 0, t;
	prev = *trafo;
	aiMultiplyMatrix4(trafo,&nd->mTransformation);
    for (; n < nd->mNumMeshes; ++n) {
#ifdef USE_ASSIMP3
        const aiMesh * mesh = scene->mMeshes[nd->mMeshes[n]];
#else
        const struct aiMesh * mesh = scene->mMeshes[nd->mMeshes[n]];
#endif
        for (t = 0; t < mesh->mNumVertices; ++t) {
#ifdef USE_ASSIMP3
            aiVector3D tmp = mesh->mVertices[t];
#else
            struct aiVector3D tmp = mesh->mVertices[t];
#endif
			aiTransformVecByMatrix4(&tmp,trafo);
			min[0] = std::min(min[0],tmp.x);
			min[1] = std::min(min[1],tmp.y);
			min[2] = std::min(min[2],tmp.z);
			max[0] = std::max(max[0],tmp.x);
			max[1] = std::max(max[1],tmp.y);
			max[2] = std::max(max[2],tmp.z);
		}
	}
	for (n = 0; n < nd->mNumChildren; ++n) {
		get_bounding_box_for_node(scene, nd->mChildren[n],min,max,trafo);
	}
	*trafo = prev;
}
		
void Scene :: getBounds(Vec3f& min, Vec3f& max) const {
#ifdef USE_ASSIMP3
    aiMatrix4x4 trafo;
#else
    struct aiMatrix4x4 trafo;
#endif
	aiIdentityMatrix4(&trafo);
	min.set(1e10f, 1e10f, 1e10f);
	max.set(-1e10f, -1e10f, -1e10f);
	get_bounding_box_for_node(mImpl->scene, mImpl->scene->mRootNode,min,max,&trafo);
}

void dumpNode(aiNode * x, std::string indent) {
	printf("%sNode (%s) with %d meshes (", indent.c_str(), x->mName.data, x->mNumMeshes);
	for (unsigned int i=0; i<x->mNumMeshes; i++) {
		printf("%d ", x->mMeshes[i]);
	}
	printf(") and %d children\n", x->mNumChildren);
	for (unsigned int i=0; i<x->mNumChildren; i++) {
		dumpNode(x->mChildren[i], indent + "\t");
	}
}
		
void Scene :: print() const {
	printf("==================================================\n");
	printf("Scene\n");
	
	printf("%d Meshes\n", meshes());
	for (unsigned int i=0; i<mImpl->scene->mNumMeshes; i++) {
		aiMesh * x = mImpl->scene->mMeshes[i];
		printf("\t%d: %s", i, x->mName.data);
		printf("\t\t%d vertices, %d faces; material: %d; normals?%d colors?%d texcoords?%d\n", x->mNumVertices, x->mNumFaces, x->mMaterialIndex, x->HasNormals(), x->HasVertexColors(0), x->HasTextureCoords(0));
		//printf("\t\tcolors: %d, normals %d, texcoords %d\n", x->mColors[0][0] != NULL, x->mNormals[0] != NULL, x->mTextureCoords[0][0] != NULL);
	}
	
	printf("%d Materials\n", materials());
	for (unsigned int i=0; i<mImpl->scene->mNumMaterials; i++) {
		aiMaterial * x = mImpl->scene->mMaterials[i];
		printf("\t%d: %d properties\n", i, x->mNumProperties);
		for (unsigned int j=0; j<x->mNumProperties; j++) {
			aiMaterialProperty * p = x->mProperties[j];
			int dim;
			std::string str;
			printf("\t\t%d: %s = { texture: %d, semantic: %d } ", j, p->mKey.data, p->mIndex, p->mSemantic);
			switch (p->mType) {
				case aiPTI_Float:
					dim = p->mDataLength/sizeof(float);
					printf("float[%d]: %f", dim, *(float *)p->mData);
					break;
				case aiPTI_String:
					str = std::string(((aiString *)p->mData)->data);
					printf("string[%d]: %s", p->mDataLength, str.c_str());
					break;
				case aiPTI_Integer:
					dim = p->mDataLength/sizeof(int);
					printf("integer[%d]: %d", dim, *(int *)p->mData);
					break;
				case aiPTI_Buffer:
					printf("buffer[%d]", p->mDataLength);
					break;
				default:
					break;
			}
			printf("\n");
			
		}
	}
	
	printf("%d Textures\n", textures());
	for (unsigned int i=0; i<mImpl->scene->mNumTextures; i++) {
		aiTexture * x = mImpl->scene->mTextures[i];
		printf("\t%d: %dx%d\n", i, x->mWidth, x->mHeight);
	}
	
	printf("%d Nodes\n", nodes());
	dumpNode(mImpl->scene->mRootNode, "");
	
	printf("==================================================\n");
}
		
		
		
