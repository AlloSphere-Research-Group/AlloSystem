/*
Allocore Example: Camera

Description:
A very simple app that shows how to build a navigable camera.

Author:
Ritesh Lala, 4/20/2011
Modified by Lance Putnam, 4/25/2011
*/

#include "allocore/io/al_Window.hpp"
#include "allocore/io/al_ControlNav.hpp"
#include "allocore/system/al_MainLoop.hpp"
#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_Lens.hpp"
#include "allocore/graphics/al_Mesh.hpp"
#include "allocore/graphics/al_Shapes.hpp"
#include "allocore/graphics/al_Stereoscopic.hpp"
#include "allocore/spatial/al_Pose.hpp"
#include "allocore/math/al_Random.hpp"

using namespace al;

struct MyWindow : public Window, public Drawable {

    MyWindow()
	:	nav(Vec3d(0,0,-2), 0.8)
	{
		cam.fovy(90);	// set field of view angle

		add(new StandardWindowKeyControls);
		add(new NavInputControl(nav));

		// create some shapes to draw
		for(int j=0; j<1000; ++j){
			int Nv = addCube(shapes);

			Mat4f xfm;
			xfm.setIdentity();
			xfm.scale(rnd::uniform(2.,0.2));
			xfm.translate(Vec3f(rnd::uniformS(20.), rnd::uniformS(20.), rnd::uniformS(20.)));
			shapes.transform(xfm, shapes.vertices().size()-Nv);

			for(int i=0; i<Nv; ++i){
				float v = float(i)/Nv;
				shapes.color(HSV(0.2*v, 1-v*0.5, 1));
			}
		}
	}

	bool onFrame(){

        nav.step();

		gl.depthTesting(true);

		Viewport vpfront(0, height()/2, width(), height()/2);
		Viewport vpback(0, 0, width(), height()/2);
		Pose posefront(nav);
		Pose poseback(nav);
		poseback.quat() *= Quatd().fromAxisY(M_PI);

		stereo.draw(gl, cam, posefront, vpfront, *this);
		stereo.draw(gl, cam, poseback, vpback, *this);

		return true;
	}

	virtual void onDraw(Graphics& gl){
		gl.draw(shapes);
	}

	virtual bool onKeyDown(const Keyboard& k){
		switch(k.key()){
			case 'f': cam.fovy(cam.fovy()-5); break;
			case 'g': cam.fovy(cam.fovy()+5); break;
			default:;
		}
		return true;
	}

    Graphics gl;
	Camera cam;
	Nav nav;
	Stereoscopic stereo;
	Mesh shapes;
};


int main(){
	MyWindow win1;
    win1.create(Window::Dim(800, 600), "Allocore Example: Camera");

	MainLoop::start();
	return 0;
}
