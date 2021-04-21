/*
Allocore Example: modelshader

Description:
This demonstrates loading an OBJ and applying a shader

Author:
Graham Wakefield 2011
*/

#include "allocore/al_Allocore.hpp"
#include "allocore/graphics/al_Shader.hpp"
#include "allocore/graphics/al_Texture.hpp"
#include "allocore/graphics/al_Image.hpp"
#include "allocore/graphics/al_Asset.hpp"
#include "allocore/graphics/al_DisplayList.hpp"

using namespace al;

Graphics gl;
SearchPaths searchpaths;

Shader vert, frag;
ShaderProgram shaderprogram;
Texture tex;
Light light;
Material material;

Scene * ascene = 0;
Vec3f scene_min, scene_max, scene_center;
DisplayList scene_list;

float a = 0.f; // current rotation angle


struct MyWindow : Window{

	bool onCreate(){

		// initialize the shaders:
		frag.compile();
		vert.compile();
		shaderprogram.attach(vert);
		shaderprogram.attach(frag);
		shaderprogram.link();
		shaderprogram.listParams();
		printf("frag %s\n", frag.log());
		printf("vert %s\n", vert.log());


		// draw the models into a display list:
		scene_list.begin();
		Mesh mesh;
			for (unsigned i=0; i<ascene->meshes(); i++) {
				ascene->mesh(i, mesh);
				gl.draw(mesh);
			}
		scene_list.end();

		return true;
	}

	bool onDestroy(){
		return true;
	}

	bool onFrame(){

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

//		gl.enable(gl.DEPTH_TEST);
////		gl.enable(gl.NORMALIZE);
//		light.twoSided(true);
//		light.dir(1.f, 1.f, 2.f);
//
//		light();
//		material();
//		glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

		gl.pushMatrix();

		// rotate it around the y axis
		gl.rotate(a, 0.f,1.f,0.f);
		a += 0.5;

		// scale the whole asset to fit into our view frustum
		float tmp = scene_max[0]-scene_min[0];
		tmp = al::max(scene_max[1] - scene_min[1],tmp);
		tmp = al::max(scene_max[2] - scene_min[2],tmp);
		tmp = 2.f / tmp;
		gl.scale(tmp);

		// center the model
		gl.translate( -scene_center );



		shaderprogram.begin();
		shaderprogram.uniform("tex0", 1);
		Graphics::error("tex0");
			tex.bind(1);
				scene_list.draw();
			tex.unbind();
		shaderprogram.end();

		gl.popMatrix();

		return true;
	}
};

MyWindow win1;

int main (int argc, char * const argv[]) {
	searchpaths.addAppPaths(argc, argv);
	searchpaths.addSearchPath(searchpaths.appPath() + "../../share");
	searchpaths.print();

	// load in a "scene"
	FilePath path = searchpaths.find("ducky.obj");
	printf("reading %s\n", path.filepath().c_str());

	ascene = Scene::import(path.filepath());
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

	win1.add(new StandardWindowKeyControls);
	win1.create(Window::Dim(640, 480));

	Image img(searchpaths.find("hubble.jpg").filepath());
	tex.allocate(img.array());

	MainLoop::start();

	return 0;
}
