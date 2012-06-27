/*
Allocore Example: Sphere Shapes

Description:
This renders a grid of spheres at various resolutions. The spheres are generated
from stacks of rings in spherical coordinates.

Author(s):
Lance Putnam, 4/27/2011
*/

#include "allocore/al_Allocore.hpp"
using namespace al;

struct MyWindow : public Window, public Drawable {
    
    MyWindow()
	:	angle(0), wireframe(false), vertexLight(true)
	{		
		add(new StandardWindowKeyControls);
		cam.fovy(45);
	}
    
	bool onFrame(){

		angle += 0.0007; if(angle > M_2PI) angle -= M_2PI;

		shapes.reset();
		shapes.primitive(gl.TRIANGLES);
		const int N = 8;
		for(int ix=3; ix<3+N; ++ix){
		for(int iy=2; iy<2+N; ++iy){
		
			float px = float(ix-3)/(N-1) * 2 - 1;
			float py = float(iy-2)/(N-1) * 2 - 1;
		
			int Nv = addSphere(shapes,1./N, ix,iy);

			Mat4f xfm;
			xfm.setIdentity();
			xfm.rotate(angle*13, 0,1).rotate(angle*17, 1,2);
			xfm.translate(Vec3f(px, py, 0));
			shapes.transform(xfm, shapes.vertices().size()-Nv);

			for(int i=0; i<Nv; ++i){
				float v = float(i)/Nv;
				shapes.color(HSV(0.2*v, 1-v*0.5, 1));
			}
		}}

		if(!vertexLight) shapes.decompress();
		shapes.generateNormals();

		gl.depthTesting(true);
		if(wireframe)	gl.polygonMode(gl.LINE);
		else			gl.polygonMode(gl.FILL);

		stereo.draw(gl, cam, Pose(Vec3d(0,0,2.8)), Viewport(width(), height()), *this);

		return true;
	}
    
	virtual void onDraw(Graphics& gl){
		gl.enable(gl.COLOR_MATERIAL);
		light.pos(4,4,1)();
		gl.draw(shapes);
	}
	
	virtual bool onKeyDown(const Keyboard& k){
				if(k.key('f')) wireframe  ^=true;
		else	if(k.key('l')) vertexLight^=true;
		return true;
	}
    
    Graphics gl;
	Camera cam;
	Stereographic stereo;
	Mesh shapes;
	Light light;
	double angle;
	bool wireframe;
	bool vertexLight;
};

MyWindow win;

int main(){
    win.create(Window::Dim(800, 600), "Allocore Example: Sphere Shapes");
	MainLoop::start();
	return 0;
}
