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

    Graphics gl;
	Lens lens;
	Nav nav{Vec3f(0,0,-2), 0.8};
	Stereoscopic stereo;
	Mesh shapes;

    MyWindow(){
		lens.fovy(90);	// set field of view angle

		append(*new StandardWindowKeyControls);
		append(*new NavInputControl(nav));

		// create some shapes to draw
		for(int j=0; j<1000; ++j){
			int Nv = addCube(shapes);

			Mat4f xfm{1};
			xfm.scale(rnd::uniform(2.,0.2));
			xfm.translate(rnd::ball<Vec3f>().normalize(20));
			shapes.transform(xfm, shapes.vertices().size()-Nv);

			for(int i=0; i<Nv; ++i){
				float v = float(i)/Nv;
				shapes.color(HSV(0.2*v, 1-v*0.5, 1));
			}
		}
	}

	bool onFrame() override {

        nav.step();

		gl.depthTesting(true);

		stereo.draw(gl, lens, nav, Viewport(width(), height()), *this);

		return true;
	}

	void onDraw(Graphics& gl) override {
		gl.draw(shapes);
	}

	bool onKeyDown(const Keyboard& k) override {
		switch(k.key()){
			case 'f': lens.fovy(lens.fovy()-5); break;
			case 'g': lens.fovy(lens.fovy()+5); break;
			default:;
		}
		return true;
	}
};


int main(){
	
	MyWindow win;
    win.create(Window::Dim(800, 600), "Allocore Example: Camera");

	MainLoop::start();
	return 0;
}
