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

using namespace al;
using namespace std;


struct MyApp : public App {
public:

	MeshVBO shapesVBO;
	MeshVBO sphereVBO;
	Mesh cubeMesh;
	Light light;
	Material material;

	int numShapes = 100000;
	float scatterSize = 1.0;
	bool useVBO = true;
	bool showFPS = false;
	int frameNum = 0;

	MyApp(){
	  initWindow(Window::Dim(800, 600), "VBO Many Shape");

		// add a sphere and cube to test = operators. '0' and '9' on keyboard
		addSphere(sphereVBO);
		sphereVBO.translate(0,100,90);
		sphereVBO.generateNormals();
		for (int i=0; i<sphereVBO.vertices().size(); i++) sphereVBO.color(0,1,0);

		addCube(cubeMesh);
		cubeMesh.translate(0,100,90);
		cubeMesh.generateNormals();
		for (int i=0; i<cubeMesh.vertices().size(); i++) cubeMesh.color(1,0,1);
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

			// Color shapes randomly
			Color randc = Color(rnd::uniform(), rnd::uniform(), rnd::uniform(0.75,1.0));
			Colori randci = Colori(rnd::uniform()*255, rnd::uniform()*255, rnd::uniform(0.75,1.0)*255);
			for(int i=0; i<Nv; ++i){
				shapesVBO.color(randci);
			}
		}

		// Convert to non-indexed triangles to get flat shading
		shapesVBO.decompress();
		shapesVBO.generateNormals();

		// Update the VBO after all calculations are completed
    if (useVBO) shapesVBO.update();
	}

  void onCreate(const ViewpointWindow& win) {
		scatterShapes();

		// initialize the VBO
		shapesVBO.allocate();
		shapesVBO.primitive(Graphics::TRIANGLES);

		sphereVBO.allocate();

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
			if (!useVBO) g.draw((Mesh&)shapesVBO);
			else g.draw(shapesVBO);
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
      else cout << "Streaming every frame" << endl;
    }

    // make a new scattering of points
    if (k.key() == 'n') {
			shapesVBO.reset();
      scatterShapes();
    }

    if (k.key() == 'p') {
			shapesVBO.print();
    }

    if (k.key() == 'f') {
      showFPS = !showFPS;
    }
		if (k.key() == '0') {
			shapesVBO = sphereVBO;
		}
		if (k.key() == '9') {
			shapesVBO = cubeMesh;
		}
		if (k.key() == ' ') {
			if (shapesVBO.hasColoris()) printf("Has coloris!\n");
			if (shapesVBO.hasColors()) printf("Has colors!\n");
		}

	}

};

int main(){
  MyApp app;
	app.start();
}
