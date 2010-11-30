#include "allocore/al_Allocore.hpp"

// @see http://assimp.svn.sourceforge.net/viewvc/assimp/trunk/samples/SimpleOpenGL/Sample_SimpleOpenGL.c?revision=827&view=markup
#include "assimp/assimp.h"
#include "assimp/aiPostProcess.h"
#include "assimp/aiScene.h"

#include "allocore/graphics/al_Shader.hpp"



using namespace al;

Graphics gl(new GraphicsBackendOpenGL);
SearchPaths searchpaths;

Shader vert, frag;
ShaderProgram shaderprogram;


// the global Assimp scene object
const struct aiScene* scene = NULL;
GLuint scene_list = 0;
struct aiVector3D scene_min, scene_max, scene_center;


// current rotation angle
static float a = 0.f;

// ----------------------------------------------------------------------------
void get_bounding_box_for_node(const struct aiNode* nd, 
		struct aiVector3D* min, struct aiVector3D* max, struct aiMatrix4x4* trafo){
	struct aiMatrix4x4 prev;
	unsigned int n = 0, t;
	prev = *trafo;
	aiMultiplyMatrix4(trafo,&nd->mTransformation);
	for (; n < nd->mNumMeshes; ++n) {
		const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
			for (t = 0; t < mesh->mNumVertices; ++t) {
			struct aiVector3D tmp = mesh->mVertices[t];
			aiTransformVecByMatrix4(&tmp,trafo);
			min->x = MIN(min->x,tmp.x);
			min->y = MIN(min->y,tmp.y);
			min->z = MIN(min->z,tmp.z);
			max->x = MAX(max->x,tmp.x);
			max->y = MAX(max->y,tmp.y);
			max->z = MAX(max->z,tmp.z);
		}
	}
	for (n = 0; n < nd->mNumChildren; ++n) {
		get_bounding_box_for_node(nd->mChildren[n],min,max,trafo);
	}
	*trafo = prev;
}

void get_bounding_box(struct aiVector3D* min, struct aiVector3D* max)
{
	struct aiMatrix4x4 trafo;
	aiIdentityMatrix4(&trafo);
	min->x = min->y = min->z =  1e10f;
	max->x = max->y = max->z = -1e10f;
	get_bounding_box_for_node(scene->mRootNode,min,max,&trafo);
}

int loadasset(std::string path)
{
	printf("loading model %s\n", path.c_str());
	// we are taking one of the postprocessing presets to avoid
	// writing 20 single postprocessing flags here.
	scene = aiImportFile(path.c_str(), aiProcessPreset_TargetRealtime_Quality);
	if (scene) {
		get_bounding_box(&scene_min,&scene_max);
		scene_center.x = (scene_min.x + scene_max.x) / 2.0f;
		scene_center.y = (scene_min.y + scene_max.y) / 2.0f;
		scene_center.z = (scene_min.z + scene_max.z) / 2.0f;
		printf("scene center %f %f %f\n", scene_center.x, scene_center.y, scene_center.z);
		return 0;
	}
	return 1;
}

// ----------------------------------------------------------------------------
void color4_to_float4(const struct aiColor4D *c, float f[4])
{
	f[0] = c->r;
	f[1] = c->g;
	f[2] = c->b;
	f[3] = c->a;
}

// ----------------------------------------------------------------------------

void set_float4(float f[4], float a, float b, float c, float d)
{
	f[0] = a;
	f[1] = b;
	f[2] = c;
	f[3] = d;
}
// ----------------------------------------------------------------------------
void apply_material(const struct aiMaterial *mtl)
{
	float c[4];

	GLenum fill_mode;
	unsigned int ret1, ret2;
	struct aiColor4D diffuse;
	struct aiColor4D specular;
	struct aiColor4D ambient;
	struct aiColor4D emission;
	float shininess, strength;
	int two_sided;
	int wireframe;
	unsigned int max;

	set_float4(c, 0.8f, 0.8f, 0.8f, 1.0f);
	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
		color4_to_float4(&diffuse, c);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, c);

	set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
		color4_to_float4(&specular, c);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);

	set_float4(c, 0.2f, 0.2f, 0.2f, 1.0f);
	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
		color4_to_float4(&ambient, c);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, c);

	set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
		color4_to_float4(&emission, c);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, c);

	max = 1;
	ret1 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);
	max = 1;
	ret2 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS_STRENGTH, &strength, &max);
	if((ret1 == AI_SUCCESS) && (ret2 == AI_SUCCESS))
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess * strength);
	else {
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
		set_float4(c, 0.0f, 0.0f, 0.0f, 0.0f);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);
	}

	max = 1;
	if(AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_ENABLE_WIREFRAME, &wireframe, &max))
		fill_mode = wireframe ? GL_LINE : GL_FILL;
	else
		fill_mode = GL_FILL;
	glPolygonMode(GL_FRONT_AND_BACK, fill_mode);

	max = 1;
	if((AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_TWOSIDED, &two_sided, &max)) && two_sided)
		glEnable(GL_CULL_FACE);
	else 
		glDisable(GL_CULL_FACE);
}

// Can't send color down as a pointer to aiColor4D because AI colors are ABGR.
void Color4f(const struct aiColor4D *color)
{
	glColor4f(color->r, color->g, color->b, color->a);
}
void recursive_render (const struct aiScene *sc, const struct aiNode* nd)
{
	int i;
	unsigned int n = 0, t;
	struct aiMatrix4x4 m = nd->mTransformation;

	// update transform
	aiTransposeMatrix4(&m);
	glPushMatrix();
	glMultMatrixf((float*)&m);

	// draw all meshes assigned to this node
	for (; n < nd->mNumMeshes; ++n) {
		const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];

		apply_material(sc->mMaterials[mesh->mMaterialIndex]);

		if(mesh->mNormals == NULL) {
			glDisable(GL_LIGHTING);
		} else {
			glEnable(GL_LIGHTING);
		}

		if(mesh->mColors[0] != NULL) {
			glEnable(GL_COLOR_MATERIAL);
		} else {
			glDisable(GL_COLOR_MATERIAL);
		}

		for (t = 0; t < mesh->mNumFaces; ++t) {
			const struct aiFace* face = &mesh->mFaces[t];
			GLenum face_mode;

			switch(face->mNumIndices) {
				case 1: face_mode = GL_POINTS; break;
				case 2: face_mode = GL_LINES; break;
				case 3: face_mode = GL_TRIANGLES; break;
				default: face_mode = GL_POLYGON; break;
			}

			glBegin(face_mode);

			for(i = 0; i < face->mNumIndices; i++) {
				int index = face->mIndices[i];
				if(mesh->mColors[0] != NULL)
					Color4f(&mesh->mColors[0][index]);
				if(mesh->mNormals != NULL) 
					glNormal3fv(&mesh->mNormals[index].x);
				glVertex3fv(&mesh->mVertices[index].x);
			}

			glEnd();
		}

	}

	// draw all children
	for (n = 0; n < nd->mNumChildren; ++n) {
		recursive_render(sc, nd->mChildren[n]);
	}

	glPopMatrix();
}

int initialized = 0;

struct MyWindow : Window{

	bool onFrame(){
	
		if (!initialized) {
			frag.create();
			vert.create();
			shaderprogram.create();
		
			frag.compile();	
			vert.compile();
			
			shaderprogram.attach(vert);
			shaderprogram.attach(frag);
			shaderprogram.link();
			
			initialized = 1;
			
			printf("compiled shaders\n");
		}
	
		gl.clearColor(0.1, 0.1, 0.1, 1);
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		gl.viewport(0,0, width(), height());
		
		gl.matrixMode(gl.PROJECTION);
		gl.loadMatrix(Matrix4d::perspective(45, aspect(), 0.1, 100));
		
		gl.matrixMode(gl.MODELVIEW);
		gl.loadMatrix(Matrix4d::lookAt(Vec3d(0,0,-4), Vec3d(0,0,0), Vec3d(0,1,0)));


//		gl.begin(gl.QUADS);
//			gl.color(1,0,0);
//			gl.vertex(-1,-1);
//			gl.color(0,1,0);
//			gl.vertex( 1,-1);
//			gl.color(0,0,1);
//			gl.vertex( 1, 1);
//			gl.color(1,1,1);
//			gl.vertex(-1, 1);		
//		gl.end();	
		
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);    // Uses default lighting parameters
		glEnable(GL_DEPTH_TEST);
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
		glEnable(GL_NORMALIZE);
		glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
		
		const GLfloat pos[]={ 0.f, 1.f, 0.f, 0.f };
		glLightfv(GL_LIGHT0, GL_POSITION, pos);
		
		glPushMatrix();
		
		// rotate it around the y axis
		glRotatef(a, 1.f,1.f,0.f);
		a += 0.1;
		
		// scale the whole asset to fit into our view frustum 
		float tmp = scene_max.x-scene_min.x;
		tmp = MAX(scene_max.y - scene_min.y,tmp);
		tmp = MAX(scene_max.z - scene_min.z,tmp);
		tmp = 1.f / tmp;
		glScalef(tmp, tmp, tmp);
		
		// center the model
		glTranslatef( -scene_center.x, -scene_center.y, -scene_center.z );
		
		
		// if the display list has not been made yet, create a new one and
		// fill it with scene contents
		if(scene_list == 0) {
			scene_list = glGenLists(1);
			glNewList(scene_list, GL_COMPILE);
			// now begin at the root node of the imported data and traverse
			// the scenegraph by multiplying subsequent local transforms
			// together on GL's matrix stack.
			recursive_render(scene, scene->mRootNode);
			glEndList();
		}
		
		shaderprogram.begin();
		glCallList(scene_list);
		shaderprogram.end();
		
		glPopMatrix();
		
		return true;
	}
};

int main (int argc, char * const argv[]) {
	searchpaths.addAppPaths(argc, argv);
	searchpaths.addSearchPath(searchpaths.appPath() + "../../resources");
	
	
	// get a handle to the predefined STDOUT log stream and attach
	// it to the logging system. It will be active for all further
	// calls to aiImportFile(Ex) and aiApplyPostProcessing.
	struct aiLogStream stream = aiGetPredefinedLogStream(aiDefaultLogStream_STDOUT,NULL);
	aiAttachLogStream(&stream);
	
	// load in a "scene"
	FilePath path = searchpaths.find("magnolia.obj");
	//FilePath path = searchpaths.find("body_cylind.obj");
	if (loadasset(path.filepath())) {
		printf("error reading %s\n", path.filepath().c_str());
		return -1;
	}
	File frag_file(searchpaths.find("shaderTestF.glsl"), "r", true);
	File vert_file(searchpaths.find("shaderTestV.glsl"), "r", true);
	
	printf("frag_file %s %s\n", frag_file.path().c_str(), frag_file.readAll());
	printf("vert_file %s %s\n", vert_file.path().c_str(), vert_file.readAll());
	
	frag.source(frag_file.readAll(), Shader::FRAGMENT);	
	vert.source(vert_file.readAll(), Shader::VERTEX);
	
	MyWindow win1;
	win1.add(new StandardWindowKeyControls);
	win1.create(Window::Dim(800, 600));

	MainLoop::start();
	
	// cleanup
	aiReleaseImport(scene);
	aiDetachAllLogStreams();
	
	return 0;
}
