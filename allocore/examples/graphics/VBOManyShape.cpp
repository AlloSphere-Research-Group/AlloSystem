/*
Allocore Example: Many Shape VBO

Description:
Modification of the meshManyShape example to render using a vertex buffer. 

Usage:
m - switch between using the VBO and immediate rendering. They should look 
		identical, but using the VBO will run much faster.
n - redo the scattering and get a new configuration
p - print information on the mesh
f - print frames per second in the console (every 10th frame)

Authors:
Lance Putnam, April 2011
Kurt Kaminski, December 2015
*/

#include "allocore/al_Allocore.hpp"
#include "alloutil/al_MeshVBO.hpp"        

using namespace al;


struct MyApp : public App {
public:

	MeshVBO shapesVBO;
	Light light;
	Material material;

	int numShapes = 100000;
	float scatterSize = 1.0;
	bool useVBO = true;
	bool showFPS = false;
	int frameNum = 0;

	MyApp(){
	  initWindow(Window::Dim(800, 600), "VBO Many Shape");
	}

	void scatterShapes(){
		for(int i=0; i<numShapes; ++i){
			int Nv = rnd::prob(0.5)
						? (rnd::prob(0.5) ? addCube(shapesVBO) : addDodecahedron(shapesVBO))
						: addIcosahedron(shapesVBO);

			// Scale and translate the newly added shape
			scatterSize = (float)numShapes * .001;
			Mat4f xfm;
			xfm.setIdentity();
			xfm.scale(Vec3f(rnd::uniform(1.,0.1), rnd::uniform(1.,0.1), rnd::uniform(1.,0.1)));
			xfm.translate(Vec3f(rnd::uniformS(scatterSize), rnd::uniformS(scatterSize), rnd::uniformS(scatterSize)));
			shapesVBO.transform(xfm, shapesVBO.vertices().size()-Nv);

			// Color newly added vertices
			for(int i=0; i<Nv; ++i){
				float f = float(i)/Nv;
				shapesVBO.color(HSV(f*0.1+0.2,1,1));
			}
		}

		// Convert to non-indexed triangles to get flat shading
		shapesVBO.decompress();
		shapesVBO.generateNormals();

		// Update the VBO after all calculations are completed
    if (useVBO) shapesVBO.updateVBO();
	}

  void onCreate(const ViewpointWindow& win) {
		// initialize the VBO
		shapesVBO.initVBO(GL_DYNAMIC_DRAW);
		
		// add a bunch of shapes to the mesh
		scatterShapes();

		// print information on the mesh
		shapesVBO.print();

		// set max framerate to 100, initial position and clipping distance
    window(0).fps(100);
		nav().pos(0,0,24);
    lens().far(300);
	}

	void onDraw(Graphics& g){
		material();

		// draw in either immediate mode or using a VBO
		g.pushMatrix();
		g.translate(0,-scatterSize,-scatterSize);
		light();
		if (!useVBO) g.draw(shapesVBO);
		else shapesVBO.draw(GL_TRIANGLES);
		g.popMatrix();
    
    frameNum++;
    if (showFPS) {
	    if (!(frameNum % 10)) cout<<"FPS: "<< window(0).fpsActual() << endl;
	  }
	}

  void onKeyDown(const Keyboard& k) {
    if (k.key() == 'm') {
      useVBO = !useVBO;
      if (useVBO) cout << "Using VBO" << endl;
      else cout << "Using immediate mode" << endl;
    }

    // make a new scattering of points
    if (k.key() == 'n') {
      scatterShapes();
    }

    if (k.key() == 'p') {
			shapesVBO.print();
    }

    if (k.key() == 'f') {
      showFPS = !showFPS;
    }
  }

};

int main(){
  MyApp app;
	app.start();
}
