#include "allocore/al_Allocore.hpp"
#include "al_Asset.hpp"

// @see http://assimp.svn.sourceforge.net/viewvc/assimp/trunk/samples/SimpleOpenGL/Sample_SimpleOpenGL.c?revision=827&view=markup
#include "assimp/assimp.h"
#include "assimp/aiPostProcess.h"
#include "assimp/aiScene.h"

#include "allocore/graphics/al_Shader.hpp"
#include "allocore/graphics/al_Texture.hpp"
#include "allocore/graphics/al_Image.hpp"

using namespace al;

GraphicsGL gl;
SearchPaths searchpaths;

Shader vert, frag;
ShaderProgram shaderprogram;
Texture tex(&gl);
Image img;

Scene * ascene = 0;
Vec3f scene_min, scene_max, scene_center;
GLuint scene_list = 0;

float a = 0.f; // current rotation angle


struct MyWindow : Window{

	bool onFrame(){
		
		// this is annoying:
		if (!shaderprogram.created()) {
			frag.compile();	
			vert.compile();
			shaderprogram.attach(vert);
			shaderprogram.attach(frag);
			shaderprogram.link();
			//shaderprogram.listParams();
			printf("frag %s\n", frag.log());
			printf("vert %s\n", vert.log());
		}
	
		gl.clearColor(0.1, 0.1, 0.1, 1);
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		gl.viewport(0,0, width(), height());
		
		gl.matrixMode(gl.PROJECTION);
		gl.loadMatrix(Matrix4d::perspective(45, aspect(), 0.1, 100));
		
		gl.matrixMode(gl.MODELVIEW);
		gl.loadMatrix(Matrix4d::lookAt(Vec3d(0,0,4), Vec3d(0,0,0), Vec3d(0,1,0)));
		
		
		gl.disable(gl.LIGHTING);
		
//		shaderprogram.begin();
//		shaderprogram.uniform("tex0", 0);
//		GraphicsGL::gl_error("shader");
//		tex.bind(0);
//		gl.begin(gl.QUADS);
//		gl.color(1, 1, 1);
//		gl.texCoord(0, 0);
//		gl.vertex(0, 0, 0);
//		gl.texCoord(0, 1);
//		gl.vertex(0, 1, 0);
//		gl.texCoord(1, 1);
//		gl.vertex(1, 1, 0);
//		gl.texCoord(1, 0);
//		gl.vertex(1, 0, 0);
//		gl.end();
//		tex.unbind(0);
//		shaderprogram.end();
		
		
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);    // Uses default lighting parameters
		glEnable(GL_DEPTH_TEST);
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
		glEnable(GL_NORMALIZE);
		glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
		
		const GLfloat pos[]={ 1.f, 1.f, 2.f, 0.f };
		glLightfv(GL_LIGHT0, GL_POSITION, pos);
		
		glPushMatrix();
		
		// rotate it around the y axis
		glRotatef(a, 0.f,1.f,0.f);
		a += 0.5;
		
		// scale the whole asset to fit into our view frustum 
		float tmp = scene_max[0]-scene_min[0];
		tmp = MAX(scene_max[1] - scene_min[1],tmp);
		tmp = MAX(scene_max[2] - scene_min[2],tmp);
		tmp = 2.f / tmp;
		glScalef(tmp, tmp, tmp);
	
		// center the model
		gl.translate( -scene_center );
		
		
		// if the display list has not been made yet, create a new one and
		// fill it with scene contents
		if(scene_list == 0) {
			scene_list = glGenLists(1);
			glNewList(scene_list, GL_COMPILE);
			
			Mesh mesh;
			for (int i=0; i<ascene->meshes(); i++) {
				ascene->mesh(i, mesh);
				gl.draw(mesh);
			}	
			
			//recursive_render(scene, scene->mRootNode);
			glEndList();
		}
		
		shaderprogram.begin();
		shaderprogram.uniform("tex", 0);
		GraphicsGL::gl_error("tex");
		tex.bind(0);
		glCallList(scene_list);
		tex.unbind(0);
		shaderprogram.end();
		
		glPopMatrix();
		
		return true;
	}
};

int main (int argc, char * const argv[]) {
	searchpaths.addAppPaths(argc, argv);
	searchpaths.addSearchPath(searchpaths.appPath() + "../../resources");
	
	// load in a "scene"
	//FilePath path = searchpaths.find("magnolia.obj");
	FilePath path = searchpaths.find("ducky.obj");
	//FilePath path = searchpaths.find("dodecahedron.obj");
	//FilePath path = searchpaths.find("body_cylind.obj");
	
	//ascene = Scene::import(path.filepath());
	ascene = Scene::import("/Users/grahamwakefield/Source/allosphere/nanomed/trunk/models/uvExportFromMaya/arteries.obj");
	if (ascene==0) {
		printf("error reading %s\n", path.filepath().c_str());
		return -1;
	} else {
		ascene->getBounds(scene_min,scene_max);
		scene_center = (scene_min + scene_max) / 2.f;
		ascene->dump();
	}
	File frag_file(searchpaths.find("basicFragment.glsl"), "r", true);
	File vert_file(searchpaths.find("basicVertex.glsl"), "r", true);
	
	printf("frag_file %s\n", frag_file.path().c_str());
	printf("vert_file %s\n", vert_file.path().c_str());
	
	frag.source(frag_file.readAll(), Shader::FRAGMENT);	
	vert.source(vert_file.readAll(), Shader::VERTEX);
	
	path = searchpaths.find("hubble.jpg");
	img.load(path.filepath());
	tex.fromArray(&img.array());
	
	MyWindow win1;
	win1.add(new StandardWindowKeyControls);
	win1.create(Window::Dim(640, 480));
	
	MainLoop::start();
	
	return 0;
}



// ----------------------------------------------------------------------------
//void color4_to_float4(const struct aiColor4D *c, float f[4])
//{
//	f[0] = c->r;
//	f[1] = c->g;
//	f[2] = c->b;
//	f[3] = c->a;
//}
//
// ----------------------------------------------------------------------------
//
//void set_float4(float f[4], float a, float b, float c, float d)
//{
//	f[0] = a;
//	f[1] = b;
//	f[2] = c;
//	f[3] = d;
//}
// ----------------------------------------------------------------------------
//void apply_material(const struct aiMaterial *mtl)
//{
//	float c[4];
//
//	GLenum fill_mode;
//	unsigned int ret1, ret2;
//	struct aiColor4D diffuse;
//	struct aiColor4D specular;
//	struct aiColor4D ambient;
//	struct aiColor4D emission;
//	float shininess, strength;
//	int two_sided;
//	int wireframe;
//	unsigned int max;
//
//	set_float4(c, 0.8f, 0.8f, 0.8f, 1.0f);
//	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
//		color4_to_float4(&diffuse, c);
//	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, c);
//
//	set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
//	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
//		color4_to_float4(&specular, c);
//	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);
//
//	set_float4(c, 0.2f, 0.2f, 0.2f, 1.0f);
//	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
//		color4_to_float4(&ambient, c);
//	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, c);
//
//	set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
//	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
//		color4_to_float4(&emission, c);
//	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, c);
//
//	max = 1;
//	ret1 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);
//	max = 1;
//	ret2 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS_STRENGTH, &strength, &max);
//	if((ret1 == AI_SUCCESS) && (ret2 == AI_SUCCESS))
//		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess * strength);
//	else {
//		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
//		set_float4(c, 0.0f, 0.0f, 0.0f, 0.0f);
//		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);
//	}
//
//	max = 1;
//	if(AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_ENABLE_WIREFRAME, &wireframe, &max))
//		fill_mode = wireframe ? GL_LINE : GL_FILL;
//	else
//		fill_mode = GL_FILL;
//	glPolygonMode(GL_FRONT_AND_BACK, fill_mode);
//
//	max = 1;
//	if((AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_TWOSIDED, &two_sided, &max)) && two_sided)
//		glEnable(GL_CULL_FACE);
//	else 
//		glDisable(GL_CULL_FACE);
//}
//
// Can't send color down as a pointer to aiColor4D because AI colors are ABGR.
//void Color4f(const struct aiColor4D *color)
//{
//	glColor4f(color->r, color->g, color->b, color->a);
//}
//void recursive_render (const struct aiScene *sc, const struct aiNode* nd)
//{
//	int i;
//	unsigned int n = 0, t;
//	struct aiMatrix4x4 m = nd->mTransformation;
//
//	// update transform
//	aiTransposeMatrix4(&m);
//	glPushMatrix();
//	glMultMatrixf((float*)&m);
//
//	// draw all meshes assigned to this node
//	for (; n < nd->mNumMeshes; ++n) {
//		const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
//
//		//apply_material(sc->mMaterials[mesh->mMaterialIndex]);
//
//		if(mesh->mNormals == NULL) {
//			glDisable(GL_LIGHTING);
//		} else {
//			glEnable(GL_LIGHTING);
//		}
//
//		if(mesh->mColors[0] != NULL) {
//			glEnable(GL_COLOR_MATERIAL);
//		} else {
//			glDisable(GL_COLOR_MATERIAL);
//		}
//
//		for (t = 0; t < mesh->mNumFaces; ++t) {
//			const struct aiFace* face = &mesh->mFaces[t];
//			GLenum face_mode;
//
//			switch(face->mNumIndices) {
//				case 1: face_mode = GL_POINTS; break;
//				case 2: face_mode = GL_LINES; break;
//				case 3: face_mode = GL_TRIANGLES; break;
//				default: face_mode = GL_POLYGON; break;
//			}
//
//			glBegin(face_mode);
//
//			for(i = 0; i < face->mNumIndices; i++) {
//				int index = face->mIndices[i];
//				if(mesh->mColors[0] != NULL)
//					Color4f(&mesh->mColors[0][index]);
//				if(mesh->mNormals != NULL) 
//					glNormal3fv(&mesh->mNormals[index].x);
//				if(mesh->mTextureCoords[0] != NULL) 
//					glTexCoord3fv(&mesh->mTextureCoords[0][index].x);
//				glVertex3fv(&mesh->mVertices[index].x);
//			}
//
//			glEnd();
//		}
//
//	}
//
//	// draw all children
//	for (n = 0; n < nd->mNumChildren; ++n) {
//		recursive_render(sc, nd->mChildren[n]);
//	}
//
//	glPopMatrix();
//}
